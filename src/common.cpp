#include "common.h"

void ConvertUtf8toAscii(const uint8_t *in, uint16_t in_len, char *out, uint16_t out_len)
{
    while ((in_len > 0) && (out_len > 1))
    {
        uint8_t ch = *in++;
        in_len--;
        if ((ch >= ' ') && (ch <= '}'))
        {
            *out++ = ch;
            out_len--;
        }
        else if ((ch == 0xc3) && (in_len > 0))
        {
            switch(*in++) 
            {
                case 0x9f:
                *out++ = 's';
                *out++ = 's';
                out_len -= 2;
                break;
                case 0xa4:
                *out++ = 'a';
                *out++ = 'e';
                out_len -= 2;
                break;
                case 0xb6:
                *out++ = 'o';
                *out++ = 'e';
                out_len -= 2;
                break;
                case 0xbc:
                *out++ = 'u';
                *out++ = 'e';
                out_len -= 2;
                break;
                case 0x84:
                *out++ = 'A';
                *out++ = 'e';
                out_len -= 2;
                break;
                case 0x96:
                *out++ = 'O';
                *out++ = 'e';
                out_len -= 2;
                break;
                case 0x9c:
                *out++ = 'U';
                *out++ = 'e';
                out_len -= 2;
                break;
                default:
                *out++ = '?';
                out_len--;
                break;
            }
            in_len--;
        }
        else
        {
            *out++ = '?';
            out_len--;
        }
    }
    *out = 0;
}

Timer timer;

void StartTimer()
{
    timer.start();
}

uint32_t GetMillis()
{
    return timer.read_ms();
}

uint32_t RoundDistance(uint32_t val)
{
    uint32_t ret = ((val + 5) / 10) * 10;
    return ret;
}
