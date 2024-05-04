# Graphic
  - Component dependency

    ![GraphicDiagram](https://github.com/nupnup-hub/JinEngine/assets/59456231/cdd52031-4b22-49fa-be97-af8a2baba886)
 
   
## Detail
  - Responsible for all functions related to graphics
  - Designed to use a variety of graphical APIs(Currently, only dx12 is applied)
  - 
  - Class
    - Graphic
      - Manage sub class
      - Control graphics-related functions at the highest level
      - Although it is in a dependency relationship with the object module, the scope of the impact is limited by referring only to a specific interface or info, option.
    - Sub class
      - Has responsible for one or more tasks 
    - Option
      - User interface to control graphics
    - Info
      - Public statistics on graphic related data    

## Specific task senario
  - Sub class creation
    
    ![GraphicSubclass](https://github.com/nupnup-hub/JinEngine/assets/59456231/67b79604-e9d5-4191-b5ba-dca5edd0a4a6)

  - Graphic resource creation

    ![GraphicResourceCreation drawio](https://github.com/nupnup-hub/JinEngine/assets/59456231/ac28268b-2541-4a3e-bbd7-82ec2408933a)

    
  - Update frame
    
    ![GraphicFrameUpdate drawio](https://github.com/nupnup-hub/JinEngine/assets/59456231/f8368c61-e04a-4199-83dc-ea79c7020305)
    
  - Draw
    
    ![GraphicDraw drawio](https://github.com/nupnup-hub/JinEngine/assets/59456231/886abff7-78c0-4520-9dd0-b26f3d57d5cf)

## GBuffer
  
  ![GBufferLayer drawio](https://github.com/nupnup-hub/JinEngine/assets/59456231/62e70476-6034-45c0-9d43-aa48895fec42)


