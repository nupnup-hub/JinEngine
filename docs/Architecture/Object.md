# Object

- Component dependency     
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

## Specific task senario
- Component Creation

![Component creation drawio](https://github.com/nupnup-hub/JinEngine/assets/59456231/59a06155-cd32-45da-9da0-5d960169643f)
    
- Resouce Destruction

 ![ResourceDestruction](https://github.com/nupnup-hub/JinEngine/assets/59456231/82c74c15-3855-4a07-8b7b-e2776c82af09)



  

