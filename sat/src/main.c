//
//  main.c
//  satellite
//
//  Created by Yujiro Miyabayashi on 13/08/31.
//  Copyright (c) 2013年 Yujiro Miyabayashi. All rights reserved.
//

#include <stdio.h>

#include "common.h"
#include "cmd.h"


int main(int argc, const char * argv[])
{
    printf("START HODOYOSHI_PI (DEVICE_ID=0x%x)\n",DEVICE_ID);

    cmd_main();
    
    return 0;
}



