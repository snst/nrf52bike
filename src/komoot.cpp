#include "komoot.h"
#include "tracer.h"
#include "val.h"

void process_komoot_data(const uint8_t *data, uint32_t len, val_uint8_t & dir, val_uint32_t & dist, uint8_t* street, uint8_t street_len)
{
    if(len >= 9) {
        dir.current = data[4];
        dist.current = data[5];
        dist.current |= data[6] <<8 ;
        dist.current |= data[7]<<16;
        dist.current |= data[8]<<24;
        memset(street, 0, street_len);
        memcpy(street, &data[9], len-9);
        INFO("komoot: dir=%u, dist=%u, len=%u, street=%s\r\n", dir.current, dist.current, len-9, street);
    }
    
}