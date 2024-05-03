# Overview
- module diagram
  
![dir_274a967828c2c6b04a4f42b9857d59f3_dep](https://github.com/nupnup-hub/JinEngine/assets/59456231/19e55b64-b7e7-407a-ad3a-1c8d7a717d09) 
 
#  The behavior of modules

## Core
- Define engine rule 

## Window
- Access and management of the operating system api

## Application
- Project management defined within the engine internals
- Access to other binary programs

## Develop
- Utility used during development

## Object
- Definition of objects constituting the scene for rendering

## Graphic
- Rendering
- Manage graphic resource

## Editor 
- Providing GUI for users to access engine resources

## Main
- Controll engine flow 
![MainFlow](https://github.com/nupnup-hub/JinEngine/assets/59456231/f79bf500-36bd-43c8-b0bb-be63ac4c9411)

  - Update wait: Wait for the command to complete to a certain point in gpu
  - Update reflection: Update reflection event ... [reflection system detail](https://github.com/nupnup-hub/JinEngine/blob/Main/docs/Architecture/TypeReflection.md)
  - Update thread: Manage enable cpu thread
  - Update editor: Do update editor process ... [editor detail](https://github.com/nupnup-hub/JinEngine/blob/Main/docs/Architecture/Graphic.md)
  - Update frame resource: Do update graphic frame resource used by scene object ... [graphic detail](https://github.com/nupnup-hub/JinEngine/blob/Main/docs/Architecture/Graphic.md)
  - draw: Scene rendering and display ... [graphic detail](https://github.com/nupnup-hub/JinEngine/blob/Main/docs/Architecture/Editor.md)
  


