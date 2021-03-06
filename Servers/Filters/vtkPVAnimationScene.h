/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkPVAnimationScene.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkPVAnimationScene - the animation scene manager.
// .SECTION Description
// vtkAnimationCue and vtkPVAnimationScene provide the framework to support
// animations in VTK. vtkAnimationCue represents an entity that changes/
// animates with time, while vtkPVAnimationScene represents scene or setup 
// for the animation, which consists of individual cues or other scenes.
//
// A scene can be played in real time mode, or as a seqence of frames
// 1/frame rate apart in time.
// .SECTION See Also
// vtkAnimationCue

#ifndef __vtkPVAnimationScene_h
#define __vtkPVAnimationScene_h

#include "vtkAnimationCue.h"

class vtkAnimationCue;
class vtkCollection;
class vtkCollectionIterator;
class vtkTimerLog;

class VTK_EXPORT vtkPVAnimationScene: public vtkAnimationCue
{
public:
  vtkTypeRevisionMacro(vtkPVAnimationScene, vtkAnimationCue);
  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkPVAnimationScene* New();

  // Description:
  // Add/Remove an AnimationCue to/from the Scene.
  // It's an error to add a cue twice to the Scene.
  void AddCue(vtkAnimationCue* cue);
  void RemoveCue(vtkAnimationCue* cue);
  void RemoveAllCues();
  int  GetNumberOfCues();
 
  // Description:
  // Sets the current animation time.
  void SetSceneTime(double time)
    { 
    this->Initialize();
    this->Tick(time, 0, time); 
    }

  // Get the time of the most recent tick.
  // The only difference between this and AnimationTime (or ClockTime) defined
  // in the superclass is that, unlike the latter this is defined even outside
  // AnimationCueTickEvent handlers.
  vtkGetMacro(SceneTime, double);

protected:
  vtkPVAnimationScene();
  ~vtkPVAnimationScene();

  // Description:
  // Called on every valid tick.
  // Calls ticks on all the contained cues.
  virtual void StartCueInternal();
  virtual void TickInternal(double currenttime, double deltatime, double clocktime);
  virtual void EndCueInternal();

  void InitializeChildren();
  void FinalizeChildren();
  
  vtkCollection* AnimationCues;
  vtkCollectionIterator* AnimationCuesIterator;
 
  double SceneTime;
private:
  vtkPVAnimationScene(const vtkPVAnimationScene&); // Not implemented.
  void operator=(const vtkPVAnimationScene&); // Not implemented.
};

#endif
