/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkDistanceWidget.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkDistanceWidget.h"
#include "vtkDistanceRepresentation2D.h"
#include "vtkCommand.h"
#include "vtkCallbackCommand.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkObjectFactory.h"
#include "vtkRenderer.h"
#include "vtkHandleWidget.h"
#include "vtkCoordinate.h"
#include "vtkHandleRepresentation.h"
#include "vtkWidgetCallbackMapper.h"
#include "vtkWidgetEvent.h"

vtkCxxRevisionMacro(vtkDistanceWidget, "$Revision: 1.11 $");
vtkStandardNewMacro(vtkDistanceWidget);


// The distance widget observes its two handles.
// Here we create the command/observer classes to respond to the 
// handle widgets.
class vtkDistanceWidgetCallback : public vtkCommand
{
public:
  static vtkDistanceWidgetCallback *New() 
    { return new vtkDistanceWidgetCallback; }
  virtual void Execute(vtkObject*, unsigned long eventId, void*)
    {
      switch (eventId)
        {
        case vtkCommand::StartInteractionEvent:
          this->DistanceWidget->StartDistanceInteraction(this->HandleNumber);
          break;
        case vtkCommand::InteractionEvent:
          this->DistanceWidget->DistanceInteraction(this->HandleNumber);
          break;
        case vtkCommand::EndInteractionEvent:
          this->DistanceWidget->EndDistanceInteraction(this->HandleNumber);
          break;
        }
    }
  int HandleNumber;
  vtkDistanceWidget *DistanceWidget;
};


//----------------------------------------------------------------------
vtkDistanceWidget::vtkDistanceWidget()
{
  this->ManagesCursor = 0;

  this->WidgetState = vtkDistanceWidget::Start;
  this->CurrentHandle = 0;

  // The widgets for moving the end points. They observe this widget (i.e.,
  // this widget is the parent to the handles).
  this->Point1Widget = vtkHandleWidget::New();
  this->Point1Widget->SetParent(this);
  this->Point2Widget = vtkHandleWidget::New();
  this->Point2Widget->SetParent(this);

  // Set up the callbacks on the two handles
  this->DistanceWidgetCallback1 = vtkDistanceWidgetCallback::New();
  this->DistanceWidgetCallback1->HandleNumber = 0;
  this->DistanceWidgetCallback1->DistanceWidget = this;
  this->Point1Widget->AddObserver(vtkCommand::StartInteractionEvent, this->DistanceWidgetCallback1, 
                                  this->Priority);
  this->Point1Widget->AddObserver(vtkCommand::InteractionEvent, this->DistanceWidgetCallback1,
                                  this->Priority);
  this->Point1Widget->AddObserver(vtkCommand::EndInteractionEvent, this->DistanceWidgetCallback1,
                                  this->Priority);

  this->DistanceWidgetCallback2 = vtkDistanceWidgetCallback::New();
  this->DistanceWidgetCallback2->HandleNumber = 1;
  this->DistanceWidgetCallback2->DistanceWidget = this;
  this->Point2Widget->AddObserver(vtkCommand::StartInteractionEvent, this->DistanceWidgetCallback2, 
                                  this->Priority);
  this->Point2Widget->AddObserver(vtkCommand::InteractionEvent, this->DistanceWidgetCallback2,
                                  this->Priority);
  this->Point2Widget->AddObserver(vtkCommand::EndInteractionEvent, this->DistanceWidgetCallback2,
                                  this->Priority);


  // These are the event callbacks supported by this widget
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
                                          vtkWidgetEvent::AddPoint,
                                          this, vtkDistanceWidget::AddPointAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MouseMoveEvent,
                                          vtkWidgetEvent::Move,
                                          this, vtkDistanceWidget::MoveAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                          vtkWidgetEvent::EndSelect,
                                          this, vtkDistanceWidget::EndSelectAction);
}

//----------------------------------------------------------------------
vtkDistanceWidget::~vtkDistanceWidget()
{
  this->Point1Widget->RemoveObserver(this->DistanceWidgetCallback1);
  this->Point1Widget->Delete();
  this->DistanceWidgetCallback1->Delete();

  this->Point2Widget->RemoveObserver(this->DistanceWidgetCallback2);
  this->Point2Widget->Delete();
  this->DistanceWidgetCallback2->Delete();
}

//----------------------------------------------------------------------
void vtkDistanceWidget::CreateDefaultRepresentation()
{
  if ( ! this->WidgetRep )
    {
    this->WidgetRep = vtkDistanceRepresentation2D::New();
    }
  reinterpret_cast<vtkDistanceRepresentation*>(this->WidgetRep)->
    InstantiateHandleRepresentation();
}

//----------------------------------------------------------------------
void vtkDistanceWidget::SetEnabled(int enabling)
{
  // The handle widgets are not actually enabled until they are placed.
  // The handle widgets take their representation from the
  // vtkDistanceRepresentation.
  if ( enabling )
    {
    if ( this->WidgetState == vtkDistanceWidget::Start )
      {
      reinterpret_cast<vtkDistanceRepresentation*>(this->WidgetRep)->
        VisibilityOff();
      }
    else
      {
      this->Point1Widget->SetEnabled(1);
      this->Point2Widget->SetEnabled(1);
      }
    }

  // Done in this weird order to get everything to work right. This invocation
  // creates the default representation.
  this->Superclass::SetEnabled(enabling);

  if ( enabling )
    {
    this->Point1Widget->SetRepresentation(
      reinterpret_cast<vtkDistanceRepresentation*>(this->WidgetRep)->
      GetPoint1Representation());
    this->Point1Widget->SetInteractor(this->Interactor);
    this->Point1Widget->GetRepresentation()->SetRenderer(this->CurrentRenderer);

    this->Point2Widget->SetRepresentation(
      reinterpret_cast<vtkDistanceRepresentation*>(this->WidgetRep)->
      GetPoint2Representation());
    this->Point2Widget->SetInteractor(this->Interactor);
    this->Point2Widget->GetRepresentation()->SetRenderer(this->CurrentRenderer);
    }
  else
    {
    this->Point1Widget->SetEnabled(0);
    this->Point2Widget->SetEnabled(0);
    }
}

// The following methods are the callbacks that the measure widget responds to
//-------------------------------------------------------------------------
void vtkDistanceWidget::AddPointAction(vtkAbstractWidget *w)
{
  vtkDistanceWidget *self = reinterpret_cast<vtkDistanceWidget*>(w);
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // Freshly enabled and placing the first point
  if ( self->WidgetState == vtkDistanceWidget::Start )
    {
    self->GrabFocus(self->EventCallbackCommand);
    self->WidgetState = vtkDistanceWidget::Define;
    self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
    reinterpret_cast<vtkDistanceRepresentation*>(self->WidgetRep)->VisibilityOn();
    double e[2];
    e[0] = static_cast<double>(X);
    e[1] = static_cast<double>(Y);
    reinterpret_cast<vtkDistanceRepresentation*>(self->WidgetRep)->StartWidgetInteraction(e);
    self->CurrentHandle = 0;
    self->InvokeEvent(vtkCommand::PlacePointEvent,(void*)&(self->CurrentHandle));
    }

  // Placing the second point is easy
  else if ( self->WidgetState == vtkDistanceWidget::Define )
    {
    self->CurrentHandle = 1;
    self->InvokeEvent(vtkCommand::PlacePointEvent,(void*)&(self->CurrentHandle));
    self->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
    self->WidgetState = vtkDistanceWidget::Manipulate;
    self->Point1Widget->SetEnabled(1);
    self->Point2Widget->SetEnabled(1);
    self->CurrentHandle = -1;
    self->ReleaseFocus();
    }

  // Maybe we are trying to manipulate the widget handles
  else //if ( self->WidgetState == vtkDistanceWidget::Manipulate )
    {
    int state = self->WidgetRep->ComputeInteractionState(X,Y);
    if ( state == vtkDistanceRepresentation::Outside )
      {
      self->CurrentHandle = -1;
      return;
      }

    self->GrabFocus(self->EventCallbackCommand);
    if ( state == vtkDistanceRepresentation::NearP1 )
      {
      self->CurrentHandle = 0;
      }
    else if ( state == vtkDistanceRepresentation::NearP2 )
      {
      self->CurrentHandle = 1;
      }
    self->InvokeEvent(vtkCommand::LeftButtonPressEvent,NULL);
    }

  // Clean up
  self->EventCallbackCommand->SetAbortFlag(1);
  self->Render();
}

//-------------------------------------------------------------------------
void vtkDistanceWidget::MoveAction(vtkAbstractWidget *w)
{
  vtkDistanceWidget *self = reinterpret_cast<vtkDistanceWidget*>(w);

  // Do nothing if in start mode or valid handle not selected
  if ( self->WidgetState == vtkDistanceWidget::Start )
    {
    return;
    }

  // Delegate the event consistent with the state
  if ( self->WidgetState == vtkDistanceWidget::Define )
    {
    int X = self->Interactor->GetEventPosition()[0];
    int Y = self->Interactor->GetEventPosition()[1];
    double e[2];
    e[0] = static_cast<double>(X);
    e[1] = static_cast<double>(Y);
    reinterpret_cast<vtkDistanceRepresentation*>(self->WidgetRep)->WidgetInteraction(e);
    self->InvokeEvent(vtkCommand::InteractionEvent,NULL);
    self->EventCallbackCommand->SetAbortFlag(1);
    }
  else //must be moving a handle, invoke a event for the handle widgets
    {
    self->InvokeEvent(vtkCommand::MouseMoveEvent, NULL);
    }

  self->WidgetRep->BuildRepresentation();
  self->Render();
}

//-------------------------------------------------------------------------
void vtkDistanceWidget::EndSelectAction(vtkAbstractWidget *w)
{
  vtkDistanceWidget *self = reinterpret_cast<vtkDistanceWidget*>(w);

  // Do nothing if outside
  if ( self->WidgetState == vtkDistanceWidget::Start ||
       self->WidgetState == vtkDistanceWidget::Define ||
       self->CurrentHandle < 0 )
    {
    return;
    }

  self->ReleaseFocus();
  self->InvokeEvent(vtkCommand::LeftButtonReleaseEvent,NULL);
  self->CurrentHandle = -1;
  self->WidgetRep->BuildRepresentation();
  self->EventCallbackCommand->SetAbortFlag(1);
  self->Render();
}

// These are callbacks that are active when the user is manipulating the
// handles of the measure widget.
//----------------------------------------------------------------------
void vtkDistanceWidget::StartDistanceInteraction(int)
{
  this->Superclass::StartInteraction();
  this->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
}

//----------------------------------------------------------------------
void vtkDistanceWidget::DistanceInteraction(int handle)
{
  double pos[3];
  if ( handle == 0 )
    {
    reinterpret_cast<vtkDistanceRepresentation*>(this->WidgetRep)->
      GetPoint1Representation()->GetDisplayPosition(pos);
    reinterpret_cast<vtkDistanceRepresentation*>(this->WidgetRep)->
      SetPoint1DisplayPosition(pos);
    }
  else
    {
    reinterpret_cast<vtkDistanceRepresentation*>(this->WidgetRep)->
      GetPoint2Representation()->GetDisplayPosition(pos);
    reinterpret_cast<vtkDistanceRepresentation*>(this->WidgetRep)->
      SetPoint2DisplayPosition(pos);
    }
  this->InvokeEvent(vtkCommand::InteractionEvent,NULL);
}

//----------------------------------------------------------------------
void vtkDistanceWidget::EndDistanceInteraction(int)
{
  this->Superclass::EndInteraction();
  this->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
}

//----------------------------------------------------------------------
void vtkDistanceWidget::SetProcessEvents(int pe)
{
  this->Superclass::SetProcessEvents(pe);

  this->Point1Widget->SetProcessEvents(pe);
  this->Point2Widget->SetProcessEvents(pe);

}

//----------------------------------------------------------------------
void vtkDistanceWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  //Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  this->Superclass::PrintSelf(os,indent);
}