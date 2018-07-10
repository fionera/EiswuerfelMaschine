typedef enum {
  UP, DOWN, BETWEEN
} ContainerPosition;

typedef enum {
  FULL, EMPTY
} ContainerFill;

typedef enum {
  ON, OFF
} CoolingUnitState;

typedef enum {
  OPEN, CLOSED
} ValveState;

typedef enum {
  NOTHING,
  FILL_CONTAINER,
  FREEZE,
  AFTER_FREEZE,
  LOOSE
} GlobalState;

