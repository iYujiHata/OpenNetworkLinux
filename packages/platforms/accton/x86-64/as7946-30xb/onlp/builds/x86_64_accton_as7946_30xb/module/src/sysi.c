/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2017 Accton Technology Corporation.
 *
 * Licensed under the Eclipse Public License, Version 1.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *        http://www.eclipse.org/legal/epl-v10.html
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the
 * License.
 *
 * </bsn.cl>
 ************************************************************
 *
 *
 *
 ***********************************************************/
#include <unistd.h>
#include <fcntl.h>

#include <onlplib/file.h>
#include <onlp/platformi/sysi.h>
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/thermali.h>
#include <onlp/platformi/fani.h>
#include <onlp/platformi/psui.h>
#include "platform_lib.h"

#include "x86_64_accton_as7946_30xb_int.h"
#include "x86_64_accton_as7946_30xb_log.h"

#define CPLD_VERSION_FORMAT "/sys/devices/platform/as7946_30xb_sys/%s"

typedef struct cpld_version {
    char *attr_name;
    int   version;
    char *description;
} cpld_version_t;

const char*
onlp_sysi_platform_get(void)
{
    return "x86-64-accton-as7946-30xb-r0";
}

int
onlp_sysi_onie_data_get(uint8_t** data, int* size)
{
    uint8_t* rdata = aim_zmalloc(256);

    if(onlp_file_read(rdata, 256, size, IDPROM_PATH) == ONLP_STATUS_OK) {
        if(*size == 256) {
            *data = rdata;
            return ONLP_STATUS_OK;
        }
    }

    aim_free(rdata);
    *size = 0;
    return ONLP_STATUS_E_INTERNAL;
}

int
onlp_sysi_oids_get(onlp_oid_t* table, int max)
{
    int i;
    onlp_oid_t* e = table;
    memset(table, 0, max*sizeof(onlp_oid_t));

    /* 8 Thermal sensors on the chassis */
    for (i = 1; i <= CHASSIS_THERMAL_COUNT; i++) {
        *e++ = ONLP_THERMAL_ID_CREATE(i);
    }

    /* 6 LEDs on the chassis */
    for (i = 1; i <= CHASSIS_LED_COUNT; i++) {
        *e++ = ONLP_LED_ID_CREATE(i);
    }

    /* 2 PSUs on the chassis */
    for (i = 1; i <= CHASSIS_PSU_COUNT; i++) {
        *e++ = ONLP_PSU_ID_CREATE(i);
    }

    /* 5 Fans on the chassis */
    for (i = 1; i <= CHASSIS_FAN_COUNT; i++) {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }

    return 0;
}

int
onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{
    int i, ret;
    cpld_version_t cplds[] = { { "mb_cpld1_ver", 0, "Mainboard-CPLD#1"},
                   { "cpu_cpld_ver", 0, "CPU-CPLD"},
                   { "fan_cpld_ver", 0, "FAN-CPLD"} };
    /* Read CPLD version
     */
    for (i = 0; i < AIM_ARRAYSIZE(cplds); i++) {
        ret = onlp_file_read_int(&cplds[i].version, 
                     CPLD_VERSION_FORMAT, 
                     cplds[i].attr_name);

        if (ret < 0) {
            AIM_LOG_ERROR("Unable to read version from \
                       CPLD(%s)\r\n", 
                       cplds[i].attr_name);
            return ONLP_STATUS_E_INTERNAL;
        }
    }

    pi->cpld_versions = aim_fstrdup("%s:%d, %s:%d, %s:%d", 
                    cplds[0].description, 
                    cplds[0].version,
                    cplds[1].description, 
                    cplds[1].version,
                    cplds[2].description, 
                    cplds[2].version);
    return ONLP_STATUS_OK;
}

void
onlp_sysi_platform_info_free(onlp_platform_info_t* pi)
{
    aim_free(pi->cpld_versions);
}

int
onlp_sysi_platform_manage_init(void)
{
    return 0;
}
