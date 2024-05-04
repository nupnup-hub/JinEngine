# Overview
- Module dependency 
  
![ModuleDependency](https://github.com/nupnup-hub/JinEngine/assets/59456231/95452b95-551f-43a1-b473-6537d9f80526)
 
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
![MainFlow](https://github.com/nupnup-hub/JinEngine/assets/59456231/3d8eb083-4424-42e5-8da6-daf5f2910fdb)

  - Update wait
    - Wait for the command to complete to a certain point in gpu
  - Update reflection
    - Update reflection event ... [reflection system detail](https://github.com/nupnup-hub/JinEngine/blob/Main/docs/Architecture/TypeReflection.md)
  - Update thread
    - Manage enable cpu thread
  - Update editor
    - Do update editor process ... [editor detail](https://github.com/nupnup-hub/JinEngine/blob/Main/docs/Architecture/Editor.md)
  - Update frame
    - Do update graphic frame resource used by scene object ... [graphic detail](https://github.com/nupnup-hub/JinEngine/blob/Main/docs/Architecture/Graphic.md)
  - draw
    - Scene rendering and display ... [graphic detail](https://github.com/nupnup-hub/JinEngine/blob/Main/docs/Architecture/Graphic.md)
  


