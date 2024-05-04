# Object

- Component dependency     
![ObjectDiagram](https://github.com/nupnup-hub/JinEngine/assets/59456231/49d66305-9e0c-449b-9671-17681e69ac2d)
 
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
- Component Register

![Component creation drawio](https://github.com/nupnup-hub/JinEngine/assets/59456231/4b7c17e3-db33-4956-8d8e-1d763d9c3b90)

- Resouce Destruction

![ResourceDestruction](https://github.com/nupnup-hub/JinEngine/assets/59456231/46203687-4c3f-4d7b-8890-d4af8db2bcbc)
  

