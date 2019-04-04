#ifndef CRC16_H
#define CRC16_H
#define LIBRARY_VERSION_CRC16_H   "0.1.1"

//static void
//rfm_apply_crc16(struct rfm12_data* rfm12, unsigned char* ptr, unsigned len);

static unsigned short rfm_crc16_update(unsigned short crc,  unsigned char b);
static unsigned short crc16( unsigned char* ptr, unsigned len  );

#endif

static unsigned short rfm_crc16_update(unsigned short  crc,  unsigned char b)
{
   unsigned char i=0;

   crc ^=  b;
   for (i = 0; i < 8; ++i) {
      if (crc & 1)
         crc = (crc >> 1) ^ 0xa001;
      else
         crc = (crc >> 1);
   }

   return crc;
}

/*
static void
rfm_apply_crc16(struct rfm12_data* rfm12, unsigned char* ptr, unsigned len)
{
   u16 i, crc16 = ~0;
   if (0 != rfm12->group_id)
      crc16 = rfm_crc16_update(~0, rfm12->group_id);
   for (i=0; i<len; i++)
      crc16 = rfm_crc16_update(crc16, ptr[i]);

   // crc16
   ptr[len] = crc16 & 0xFF;
   ptr[len+1] = (crc16 >> 8) & 0xFF;
}
*/

static unsigned short crc16( unsigned char* ptr, unsigned   len)
{
   unsigned short  i, crc16 = ~0;

   for (i=0; i<len; i++)
      crc16 = rfm_crc16_update(crc16, ptr[i]);

   // crc16
   //ptr[len] = crc16 & 0xFF;
   //ptr[len+1] = (crc16 >> 8) & 0xFF;
   return (crc16);
}
