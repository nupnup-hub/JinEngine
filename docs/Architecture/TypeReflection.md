# Type Reflection
 
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
- Consists list structure(child own parent pointer)
- Use static structure construction func for register

## Class overview
- JTypeInfo
    - Own type per data 
- JMethodInfo
    - Own method callable 
    - Own method parameter and return type hint
- JParameterInfo
    - Own parameter get set callable 
    - Own parameter return type hint
- JReflectionInfo
    - Own typeinfo
    - Manage type per event


