# Object

- Component diagram    
![ObjectDiagram](https://github.com/nupnup-hub/JinEngine/assets/59456231/a92badc9-d3cb-42cb-89f8-b5afb1b0c7ee)
 
## Detail 
- GameObject
  - A unit of the object drawn in scene
  - Own component
  - Manage component
- Component
  - Provides a variety of features to Game Object( render, animation, light, camera...)
- Resource
  - Resources used by Engine
  - Converted to fit the engine
- Directory
  - Own and manage the file that owns metadata on the Resource
  - Depending on the frequency of use, resources exist in real memory or as metadata in file data

## Use case
- Component Creation
  - ![Component creation drawio](https://github.com/nupnup-hub/JinEngine/assets/59456231/844be91f-ef00-4ac8-8479-6690af79003c)
- Resouce Destruction
- 

