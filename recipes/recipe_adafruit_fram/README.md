A recipe to use the Adafruit FRAM, for example: https://www.adafruit.com/product/4719 .

Initially installing the FRAM adafruit library does not allow to compile for Artemis, due to some missing pin mappings; see issue https://github.com/sparkfun/Arduino_Apollo3/issues/390 .

To fix this, a simple solution is to disable the constructors that require pin mapping in both the FRAM and busIO libraries from Adafruit. This is done for example on these forks of their repos (make sure to use the fix branches!):

- https://github.com/jerabaul29/Adafruit_BusIO/tree/fix/no_mapping_needed
- https://github.com/jerabaul29/Adafruit_BusIO/tree/fix/SPI_with_Artemis
