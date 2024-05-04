# Type Reflection
 
## Class overview
![TypeReflection](https://github.com/nupnup-hub/JinEngine/assets/59456231/b338aeb3-2d38-4b07-b40f-19f1d8f2a182)

## Detail
- Define unique data per type
    - Identification(name, guid)
    - Method and parameter information
    - Instance data
    - Memory allocater
    - Initialize function
    - Lazy destruction
- Can access type unique data with interface pointer easily
- Can access registered parameter or method callable by name
- Consists list structure(child type info own parent type info pointer)
- Use static structure construction func for register


