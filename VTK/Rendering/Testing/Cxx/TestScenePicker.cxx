/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: TestScenePicker.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// 
// Test class vtkScenePicker.
// Move your mouse around the scene and the underlying actor should be 
// printed on standard output.
//

#include "vtkSphereSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkCommand.h"
#include "vtkTestUtilities.h"
#include "vtkRegressionTestImage.h"
#include "vtkScenePicker.h"
#include "vtkVolume16Reader.h"
#include "vtkProperty.h"
#include "vtkPolyDataNormals.h"
#include "vtkContourFilter.h"
#include <vtkstd/map>
#include <vtkstd/string>

//-----------------------------------------------------------------------------
// Create a few actors first
vtkActor *CreateActor1( int argc, char *argv[], vtkRenderer * aRenderer )
{
  char* fname2 = vtkTestUtilities::ExpandDataFileName(
      argc, argv, "Data/headsq/quarter");
  vtkVolume16Reader *v16 = vtkVolume16Reader::New();
    v16->SetDataDimensions (64,64);
    v16->SetImageRange (1,93);
    v16->SetDataByteOrderToLittleEndian();
    v16->SetFilePrefix (fname2);
    v16->SetDataSpacing (3.2, 3.2, 1.5);

  // An isosurface, or contour value of 500 is known to correspond to the
  // skin of the patient. Once generated, a vtkPolyDataNormals filter is
  // is used to create normals for smooth surface shading during rendering.
  vtkContourFilter *skinExtractor = vtkContourFilter::New();
    skinExtractor->SetInputConnection(v16->GetOutputPort());
    skinExtractor->SetValue(0, 500);
  vtkPolyDataNormals *skinNormals = vtkPolyDataNormals::New();
    skinNormals->SetInputConnection(skinExtractor->GetOutputPort());
    skinNormals->SetFeatureAngle(60.0);
  vtkPolyDataMapper *skinMapper = vtkPolyDataMapper::New();
    skinMapper->SetInputConnection(skinNormals->GetOutputPort());
    skinMapper->ScalarVisibilityOff();
  vtkActor *skin = vtkActor::New();
    skin->SetMapper(skinMapper);
    skin->GetProperty()->SetColor(0.95,0.75,0.75);

  aRenderer->AddActor(skin);

  v16->Delete();
  skinExtractor->Delete();
  skinNormals->Delete();
  skinMapper->Delete();
  skin->Delete();
  delete [] fname2;
  return skin;
}

//-----------------------------------------------------------------------------
// Create a few actors first
vtkActor * CreateActor2( int vtkNotUsed(argc), char **vtkNotUsed(argv), vtkRenderer * ren )
{
  vtkSphereSource *ss = vtkSphereSource::New();
  ss->SetThetaResolution(30);
  ss->SetPhiResolution(30);
  ss->SetRadius(150.0);
  vtkPolyDataMapper *mapper = vtkPolyDataMapper::New();
  mapper->SetInput(ss->GetOutput());
  vtkActor *actor = vtkActor::New();
  actor->SetMapper(mapper); 
  actor->GetProperty()->SetColor(0.0,1.0,0.0);
  ren->AddActor(actor);
  ss->Delete();
  mapper->Delete();
  actor->Delete();
  return actor;
}

//-----------------------------------------------------------------------------
// Command to write out picked actors during mouse over.
class TestScenePickerCommand : public vtkCommand
{
public:
  vtkScenePicker * m_Picker;
  static TestScenePickerCommand *New()
    {
    return new TestScenePickerCommand;
    }

  virtual void Execute(vtkObject *caller, unsigned long vtkNotUsed(eventId), void* vtkNotUsed(callData))
    {
    vtkRenderWindowInteractor * iren = reinterpret_cast<
      vtkRenderWindowInteractor *>(caller);
    int e[2];
    iren->GetEventPosition(e);
    cout << "DisplayPosition : (" << e[0] << "," << e[1] << ")"
         << " Prop: "     << m_ActorDescription[m_Picker->GetViewProp(e)].c_str()
         << " CellId: "   << m_Picker->GetCellId(e) 
         << " VertexId: " << m_Picker->GetVertexId(e)
         << endl;
    }
  
  void SetActorDescription( vtkProp *a, vtkstd::string s )
    {
    this->m_ActorDescription[a] = s;
    }
  vtkstd::map< vtkProp *, vtkstd::string > m_ActorDescription;

protected:
  TestScenePickerCommand() { this->SetActorDescription((vtkActor*)NULL, "None"); }
  virtual ~TestScenePickerCommand() {}
};

//-----------------------------------------------------------------------------
int TestScenePicker(int argc, char* argv[])
{
  vtkRenderer *ren = vtkRenderer::New();
  vtkRenderWindow *renWin = vtkRenderWindow::New();
  vtkRenderWindowInteractor *iren = vtkRenderWindowInteractor::New();

  renWin->AddRenderer(ren);
  iren->SetRenderWindow(renWin);
  ren->SetBackground(0.0, 0.0, 0.0);
  renWin->SetSize(300, 300);
   
  // Here comes the scene picker stuff. [ Just 2 lines ]
  vtkScenePicker * picker = vtkScenePicker::New();
  picker->SetRenderer(ren);

  // Write some stuff for interactive stuff.
  TestScenePickerCommand * command =
    TestScenePickerCommand::New();
  command->m_Picker = picker;
  command->SetActorDescription( CreateActor1( argc, argv, ren ), "Head"   );
  command->SetActorDescription( CreateActor2( argc, argv, ren ), "Sphere" );
  iren->AddObserver(vtkCommand::MouseMoveEvent, command);

  renWin->Render();
  iren->Initialize();

  // Check if scene picking works.
  int retVal = EXIT_SUCCESS;
  int e[2] = {175, 215};
  if (command->m_ActorDescription[picker->GetViewProp(e)] != "Head" ||
      picker->GetCellId(e) != 50992)
    {
    retVal = EXIT_FAILURE;
    }

  for ( int i = 0; i < argc; ++i )
    {
    if ( ! strcmp( argv[i], "-I" ) )
      {
      iren->Start();
      }
    }

  // Cleanups
  command->Delete();
  picker->Delete();
  ren->Delete();
  renWin->Delete();
  iren->Delete();
  
  return retVal;
}
