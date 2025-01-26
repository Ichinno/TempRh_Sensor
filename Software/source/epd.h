#ifndef EPD_H
#define EPD_H
#include "base_types.h"

#define WIDTH 152
#define HEIGHT 152
#define BYTES_PER_ROW (WIDTH / 8)

void EPD_initWft0154cz17(boolean_t isfull);
void EPD_poweroff(void);
void EPD_display(unsigned char data[][BYTES_PER_ROW]);

#endif // EPD_H
