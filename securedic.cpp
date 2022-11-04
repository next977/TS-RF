#include "securedic.h"
#include <unistd.h>


//#define mSleep(n) usleep(n*1000)
#define getch()

FILE *DFile = NULL;


extern int owdebug;



securedic::securedic(QObject *parent, SysParm *sysParm) : QObject(parent)
{
    this->sysParm = sysParm;
    printf("--------------------------------------------------\n");
    printf("DS28E25, DS28E22 and DS28E15 Demo, Version 1.3\n");
    printf("--------------------------------------------------\n\n");


    DFile = fopen("/app/app/securedIC_log.txt","a+");
    if(DFile == NULL)
    {
       printf("ERROR, Could not open LOT.TXT log file!\n");
       exit(1);
    }
}

securedic::~securedic()
{

}
void securedic::slot_SecuredIC_Start()
{
    int i,rslt,rt,device_num_pages = TRUE;
    uchar buf[256], challenge[32], manid[2], partial[32], new_page[32];

    uchar memimage0[32],memimage1[32];

    rt = TRUE;
    // Connect to I2C interface
    if(sysParm->mRfDeviceFactors->SIC_result == false)
    {
        DS2465_detect();
    }
     // power up 1-Wire
     OWPowerUp();

     rt = TRUE;
     OWOverdriveSkipROM();


    // read personality bytes to get manufacturer ID
    if (ReadStatus(TRUE, FALSE, 0, buf))
    {
       manid[0] = buf[3];
       manid[1] = buf[2];
    }
    else
    {
       rt = FALSE;
    }
    //qDebug("SIC ReadStatus, rt = %d",rt);
    //dprintf("\n DS2465 Read Manufacturer ID Success \n");
    if(rt == TRUE) // read manufacturer ID Success
    {
        if(sysParm->mRfDeviceFactors->SIC_result == false)
        {
           rslt = readPage(0, &memimage0[0 * 32], FALSE); // page 0
           if (!rslt) rt = FALSE;

           for (i = 0; i < 32; i++)
           {
               sysParm->mRfDeviceFactors->SIC_page0[i] = memimage0[i];
               //qDebug("SIC_page0[ %d ] = %d", i ,sysParm->mRfDeviceFactors->SIC_page0[i]);
           }
           sysParm->mRfDeviceFactors->SIC_result = true;
        }
        else
        {
            OWPowerDown();
            return ;
        }


    }
    else // read manufacturer ID Fail
    {

        sysParm->mRfDeviceFactors->SIC_result = false;
        sysParm->mRfDeviceFactors->SIC_Parsing = true;
        OWPowerDown();
        return ;
    }

}
#if 0
void securedic::slot_SecuredIC_Start()
{
    int i,rslt,rt,block,device_num_pages,skip_setup=TRUE;
    char comstr[50];
    uchar buf[256], challenge[32], manid[2], partial[32], new_page[32];
    int current_secret=0;
    //uchar memimage0[512],memimage1[512] , page0_data[512], page1_data[512];
    uchar memimage0[32],memimage1[32];
    uchar master_secret[32];
    uchar binding[32];
    uchar protimage[16];
    int pg;

    rt = TRUE;
    // Connect to I2C interface
    if (!gpioTools::SecuredIC_Open())
    {
        dprintf("Failed to connect I2C\n");
       exit(1);
    }
    else
       dprintf("\nDS2465 I2C detected\n");


    // verify DS2465 is present on default address
    if (!DS2465_detect())
    {
       dprintf("\nFailed to find and setup DS2465\n");
       dprintf("\nPress and key to end demo...");
       getch();
       exit(0);
    }
    else
       dprintf("\nFound DS2465\n");


    // Set DS2465 1-Wire timing:
    // tMSP(STD) = 64us, tMSP(OV) = 8.5us, tW0L(OD)=8.5us, tREC0=7.5us, tW1L(OD)=1.25us
    dprintf("DS2465 - Set 1-Wire timing\n");
    rslt = DS2465_OWConfigureTMSP(FALSE, 0x07, 0x04);
    if (!rslt) rt = FALSE;
    rslt = DS2465_OWConfigureTW0L(FALSE, 0x07, 0x06);
    if (!rslt) rt = FALSE;
    rslt = DS2465_OWConfigureTREC0(FALSE, 0x07, 0x07);
    if (!rslt) rt = FALSE;
    rslt = DS2465_OWConfigureTW1L_Overdrive(FALSE, 0x05);
    if (!rslt) rt = FALSE;
    dprintf(" %s\n\n",(rt) ? "SUCCESS" : "FAIL");


    // hard code master secret
    for (i = 0; i < 32; i++)
       master_secret[i] = i;

    // hard code binding data to make unique secret
    for (i = 0; i < 32; i++)
       binding[i] = 32-i;

    // hard code partial secret data to make unique secret
    for (i = 0; i < 32; i++)
       partial[i] = 32+i;


      // power up 1-Wire
      OWPowerUp();

      rt = TRUE;
      // Change to overdrive
      dprintf("Change speed on all connected devices to Overdrive\n");
      OWOverdriveSkipROM();
      rt = Find_DS28E25_DS28E22_DS28E15(&device_num_pages);
#if 1
    // read personality bytes to get manufacturer ID
    if (ReadStatus(TRUE, FALSE, 0, buf))
    {
       manid[0] = buf[3];
       manid[1] = buf[2];
       rt = TRUE;
       sysParm->mRfDeviceFactors->SIC_result = true;
    }
    else
    {
        rt = FALSE;
    }

    if(rt == TRUE) // read manufacturer ID Success
    {

           dprintf("\n DS2465 Read Manufacturer ID Success \n");
           //sysParm->mRfDeviceFactors->SIC_result = false;
    #if 1
          dprintf("Read-Authenticated page 0\n");
          //getRandom(challenge,32);  // random challenge
          //rslt = readAuthVerify(current_secret, 0, challenge, &memimage0[0], manid, FALSE, 0);
          rslt = readPage(0, &memimage0[0 * 32], FALSE); // page 0
          dprintf("%s\n",(rslt) ? " SUCCESS" : "FAIL");
          if (!rslt) rt = FALSE;
    #endif
    #if 0
          dprintf("Read-Authenticated page 0\n");
          getRandom(challenge,32);  // random challenge
          rslt = readAuthVerify(current_secret, 0, challenge, &memimage0[0], manid, FALSE, 0);
          dprintf("%s\n",(rslt) ? " SUCCESS" : "FAIL");
          if (!rslt) rt = FALSE;
    #endif

          // change data block 0 by inverting the current data
          //dprintf("\nPage0 data: \n");
          //dprintf("\nPage1 data: \n");
    #if 1

          for (i = 0; i < 32; i++)
          {
              sysParm->mRfDeviceFactors->SIC_page0[i] = memimage0[i];
          }

    #endif
          dprintf("\n Page Read Success \n");


    }
    else // read manufacturer ID Fail
    {

        sysParm->mRfDeviceFactors->SIC_result = false;
        OWPowerDown();
        gpioTools::SecuredIC_Close();
        return ;
    }

#endif
      OWPowerDown();
      gpioTools::SecuredIC_Close();
}
#endif

#if 0
void securedic::slot_SecuredIC_Start()
{
    int i,rslt,rt,block,device_num_pages,skip_setup=TRUE;
    char comstr[50];
    uchar buf[256], challenge[32], manid[2], partial[32], new_page[32];
    int current_secret=0;
    //uchar memimage0[512],memimage1[512] , page0_data[512], page1_data[512];
    uchar memimage0[32],memimage1[32];
    uchar master_secret[32];
    uchar binding[32];
    uchar protimage[16];
    int pg;

    rt = TRUE;
    // Connect to I2C interface
    if (!gpioTools::SecuredIC_Open())
    {
        dprintf("Failed to connect I2C\n");
       exit(1);
    }
    else
       dprintf("\nDS2465 I2C detected\n");


    // verify DS2465 is present on default address
    if (!DS2465_detect())
    {
       dprintf("\nFailed to find and setup DS2465\n");
       dprintf("\nPress and key to end demo...");
       getch();
       exit(0);
    }
    else
       dprintf("\nFound DS2465\n");


    // Set DS2465 1-Wire timing:
    // tMSP(STD) = 64us, tMSP(OV) = 8.5us, tW0L(OD)=8.5us, tREC0=7.5us, tW1L(OD)=1.25us
    dprintf("DS2465 - Set 1-Wire timing\n");
    rslt = DS2465_OWConfigureTMSP(FALSE, 0x07, 0x04);
    if (!rslt) rt = FALSE;
    rslt = DS2465_OWConfigureTW0L(FALSE, 0x07, 0x06);
    if (!rslt) rt = FALSE;
    rslt = DS2465_OWConfigureTREC0(FALSE, 0x07, 0x07);
    if (!rslt) rt = FALSE;
    rslt = DS2465_OWConfigureTW1L_Overdrive(FALSE, 0x05);
    if (!rslt) rt = FALSE;
    dprintf(" %s\n\n",(rt) ? "SUCCESS" : "FAIL");


    // hard code master secret
    for (i = 0; i < 32; i++)
       master_secret[i] = i;

    // hard code binding data to make unique secret
    for (i = 0; i < 32; i++)
       binding[i] = 32-i;

    // hard code partial secret data to make unique secret
    for (i = 0; i < 32; i++)
       partial[i] = 32+i;


      // power up 1-Wire
      OWPowerUp();

      rt = TRUE;
      // Change to overdrive
      dprintf("Change speed on all connected devices to Overdrive\n");
      OWOverdriveSkipROM();
      rt = Find_DS28E25_DS28E22_DS28E15(&device_num_pages);
#if 1
    // read personality bytes to get manufacturer ID  
    if (ReadStatus(TRUE, FALSE, 0, buf))
    {
       manid[0] = buf[3];
       manid[1] = buf[2];
       rt = TRUE;
       sysParm->mRfDeviceFactors->SIC_result = true;
    }
    else
    {
        rt = FALSE;
        sysParm->mRfDeviceFactors->SIC_result = false;
    }

    if(rt == TRUE) // read manufacturer ID Success
    {
       if(sysParm->mRfDeviceFactors->SIC_result == true)
       {
           dprintf("\n DS2465 Read Manufacturer ID Success \n");
           sysParm->mRfDeviceFactors->SIC_result = false;
    #if 1
          dprintf("Read-Authenticated page 0\n");
          //getRandom(challenge,32);  // random challenge
          //rslt = readAuthVerify(current_secret, 0, challenge, &memimage0[0], manid, FALSE, 0);
          rslt = readPage(0, &memimage0[0 * 32], FALSE); // page 0
          dprintf("%s\n",(rslt) ? " SUCCESS" : "FAIL");
          if (!rslt) rt = FALSE;
    #if 0
          dprintf("Read-Authenticated page 1\n");
          getRandom(challenge,32);  // random challenge
          rslt = readAuthVerify(current_secret, 1, challenge, &memimage1[0], manid, FALSE, 0);
          dprintf("%s\n",(rslt) ? " SUCCESS" : "FAIL");
          if (!rslt) rt = FALSE;
    #endif

          // change data block 0 by inverting the current data
          //dprintf("\nPage0 data: \n");
          //dprintf("\nPage1 data: \n");
    #if 1
          if(memimage0[0] == 'S' &&
             memimage0[1] == 'T' &&
             memimage0[2] == 'N' &&
             memimage0[3] == 'D' )
          {
              for (i = 0; i < 32; i++)
              {
                  sysParm->mRfDeviceFactors->SIC_page0[i] = memimage0[i];
                  //sysParm->mRfDeviceFactors->SIC_page1[i] = memimage1[i];


                  //page0_data[i] = memimage0[i];
                  //dprintf("%02X ",sysParm->mRfDeviceFactors->SIC_page0[i]);
                  //dprintf("%02X ",sysParm->mRfDeviceFactors->SIC_page1[i]);
              }
          }
    #endif
          dprintf("\n Page Read Success \n");
       }

    }
    else // read manufacturer ID Fail
    {

        sysParm->mRfDeviceFactors->SIC_result = false;
        gpioTools::SecuredIC_Close();
        return ;
    }

#endif
#if 0
      dprintf("Set the master secret in the Software SHA-256\n");
      set_secret(master_secret);

      dprintf("Create Unique Secret using Software SHA-256\n");
      rslt = CalculateNextSecret256(binding, partial,0,manid);
      dprintf("%s\n",(rslt) ? " SUCCESS" : "FAIL");
      if (!rslt) rt = FALSE;
#endif





#endif


#if 0
      dprintf("\nNew data: ");
      for (i = 0; i < 4; i++)
      {
         new_page[i] = ~memimage[i];
         dprintf("%02X ",new_page[i]);
      }
#endif
#if 0
      // Write new block
      dprintf("\nWrite block at address 0000h\n");
      rslt = writeAuthBlock(0, 0, &new_page[0], &memimage[0], manid, FALSE);
      dprintf("%s\n",(rslt) ? " SUCCESS" : "FAIL");
      if (!rslt) rt = FALSE;
      memcpy(&memimage[0], &new_page[0], 4);
#endif
#if 0
      /*
      uchar Page0_data[] = {
                             0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
                             0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
                             0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
                             0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00
                           };
      uchar Page1_data[] = {
                             0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
                             0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
                             0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
                             0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00
                           };
      */
      for(int i = 0; i < 32 ; i++)
      {
        page0_data[i] ++;
        page1_data[i] ++;
      }
      for(int block = 0 ; block < 8 ; block ++)
      {
          // Write new block
          //dprintf("\nblock = %d\n", block);
          //dprintf("Page0_data[%d] = 0x%02Xh\n",block, Page0_data[block]);
          rslt = writeBlock(0, block, &page0_data[block*4], FALSE);
          dprintf("\nWrite Page0 Data %d : %s\n",block, (rslt) ? " SUCCESS" : "FAIL");
          if (!rslt) rt = FALSE;
          rslt = writeBlock(1, block, &page1_data[block*4], FALSE);
          dprintf("Write Page1 Data %d : %s\n",block, (rslt) ? " SUCCESS" : "FAIL");
          if (!rslt) rt = FALSE;

          //memcpy(&memimage[0], &new_page[0], 4);
      }

#endif
#if 0
      // verify memory is correct by doing just a compute
      dprintf("Read-Authenticate page 0 after write\n");
      getRandom(challenge,32);  // random challenge
      rslt = readAuthVerify(current_secret, 0, challenge, &memimage[0], manid, TRUE, FALSE);
      dprintf("Read-Authenticate : %s\n",(rslt) ? " SUCCESS" : "FAIL");
      if (!rslt) rt = FALSE;

      // power down 1-Wire after complete
      OWPowerDown();

      dprintf("DS28E25/DS28E22/DS28E15 Application Example: %s\n",(rt) ? " SUCCESS" : "FAIL");
      dprintf("--------------------------------------------------------\n");
#endif
      OWPowerDown();
      gpioTools::SecuredIC_Close();
}
#endif
//---------------------------------------------------------------------------
// Search and find the attached 1-Wire device DS28E25, DS28E22, or DS28E15.
//
// num_pages - integer pointer to indicate number of pages on found device.
//
// Return: 1 - Success, device found
//         0 - failure to find device
//
int securedic::Find_DS28E25_DS28E22_DS28E15(int *num_pages)
{
   int i, rt = TRUE;

   dprintf("Find the connected DS28E25, DS28E22, or DS28E15\n");
   // find first device on 1-Wire network
   if (OWFirst())
   {
      if ((ROM_NO[0] & 0x7F) == DS28E25_FAMILY)
      {
         dprintf("DS28E25 Found: ");
         for (i = 0; i < 8; i++)
            dprintf("%02X ", ROM_NO[i]);
         dprintf("\n");
         *num_pages = DS28E25_PAGES;
      }
      else if ((ROM_NO[0] & 0x7F) == DS28E22_FAMILY)
      {
         dprintf("DS28E22 Found: ");
         for (i = 0; i < 8; i++)
            dprintf("%02X ", ROM_NO[i]);
         dprintf("\n");
         *num_pages = DS28E22_PAGES;
      }
      else if ((ROM_NO[0] & 0x7F) == DS28E15_FAMILY)
      {
         dprintf("DS28E15 Found: ");
         for (i = 0; i < 8; i++)
            dprintf("%02X ", ROM_NO[i]);
         dprintf("\n");
         *num_pages = DS28E15_PAGES;
      }
      else
      {
         dprintf("DS28E25, DS28E22, or DS28E15 NOT Found\n");
         rt = FALSE;
      }
   }
   else
      rt = FALSE;

   return rt;
}
/*
//--------------------------------------------------------------------------
//  Description:
//     Delay for at least 'len' ms
//
void securedic::msDelay(int len)
{
   int i;

   if (owdebug)
      dprintf("\n<Delay for %dms>",len);

   // special case while setting up adapter, long power up delay
   if (len > 100)
   {
      for (i = 0; i < len/100; i++)
      {
         mSleep(100);
         dprintf(".");
      }
   }
   else
      mSleep(len);
}
*/
//--------------------------------------------------------------------------
// Debug printf to opened file DFile with a force flush
//
// Return: number of characters printed
//
int securedic::dprintf(char *format, ...)
{
   int rt;
   va_list ap;

   if (DFile != NULL)
   {
      va_start(ap, format);
      rt = vfprintf(DFile,format,ap);
      va_end(ap);

      fflush(DFile);
   }

   va_start(ap, format);
   rt = vfprintf(stdout,format,ap);
   va_end(ap);

   return rt;
}


//---------------------------------------------------------------------------
// getRandom - create a random number and fill in provided buffer
//
// NOTE: rand() is not a great source of random numbers but adequate for
//       demonstration purposes.
//
void securedic::getRandom(uchar* buffer, int cnt)
{
   int i;

   for(i=0; i<cnt; i++)
   {
      double r = ( (double)rand() / ((double)(RAND_MAX)+(double)(1)) );
      buffer[i] = (uchar)(r*256);
   }
}



