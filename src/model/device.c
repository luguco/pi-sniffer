#include "device.h"
#include "utility.h"
#include "cJSON.h"
#include <glib.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>

// These must be in same order as enum values
char* categories[] = { "unknown", "phone", "wearable", "tablet", "headphones", "computer", 
    "tv", "fixed", "beacon", "car", "audio", "lighting", "sprinklers", "sales", "appliance", "security", "fitness", "printer",
    "speakers", "camera", "watch" };

int category_values[] = { CATEGORY_UNKNOWN, CATEGORY_PHONE, CATEGORY_WEARABLE, CATEGORY_TABLET, CATEGORY_HEADPHONES, CATEGORY_COMPUTER, 
    CATEGORY_TV, CATEGORY_FIXED, CATEGORY_BEACON, CATEGORY_CAR, CATEGORY_AUDIO_CARD, CATEGORY_LIGHTING, CATEGORY_SPRINKLERS, CATEGORY_POS, 
    CATEGORY_APPLIANCE, CATEGORY_SECURITY, CATEGORY_FITNESS, CATEGORY_PRINTER, CATEGORY_SPEAKERS, CATEGORY_CAMERA, CATEGORY_WATCH };

int category_to_int(char* category)
{
    for (uint8_t i = 0; i < sizeof(categories); i++) {
       if (category_values[i] != (int)i) g_info("Category does not match %i %i", i, category_values[i]);
       if (strcmp(category, categories[i]) == 0) return category_values[i];
    }
    return 0;
}

char* category_from_int(uint8_t i)
{
  if (i >= sizeof(categories)) return categories[0];
  return categories[i];
}

/*
   merge
*/
void merge(struct Device* local, struct Device* remote, char* access_name, bool safe)
{
    local->is_training_beacon = local->is_training_beacon || remote->is_training_beacon;

    if (strcmp(local->name, remote->name) != 0 
        && strlen(local->name)>0 && strlen(remote->name)>0
        && remote->name[0] != '_')
    {
        if (local->is_training_beacon)
        {
            g_info("Training beacon changed name '%s' to '%s' because %s", local->name, remote->name, access_name);
            g_strlcpy(local->name, remote->name, NAME_LENGTH);
        }
        else{
            g_warning("Remote changed name '%s' by remote '%s' IGNORE?", local->name, remote->name);
            g_strlcpy(local->name, remote->name, NAME_LENGTH);
        }
    }
    // TODO: All the NAME rules should be applied here too (e.g. privacy)

    optional_set(local->name, remote->name, NAME_LENGTH);
    optional_set(local->alias, remote->alias, NAME_LENGTH);
    soft_set_8(&local->addressType, remote->addressType);

    // If this is an update for an existing value update the superseded field
    if (local->supersededby != remote->supersededby)
    {
        int timedelta = difftime(remote->latest, local->latest);
        if (timedelta == 0 || safe)
        {
            local->supersededby = remote->supersededby;
        }
        else
        {
            char macsuperlocal[18];
            char macsuperremote[18];
            mac_64_to_string(macsuperlocal, 18, local->supersededby);
            mac_64_to_string(macsuperremote, 18, remote->supersededby);
            g_warning("Did not update superseded for %s from %s to %s (%i, %i)", local->mac, macsuperlocal, macsuperremote, timedelta, safe);
        }
    }

    if (remote->category != CATEGORY_UNKNOWN)
    {
        if (local->category == CATEGORY_UNKNOWN)
        {
            g_info("  %s Set category to '%s', message from %s", local->mac, category_from_int(remote->category), access_name);
            local->category = remote->category;
        }
        else if (local->category == CATEGORY_PHONE && (remote->category == CATEGORY_TABLET || remote->category == CATEGORY_WATCH))
        {
            // TABLET/WATCH overrides PHONE because we assume phone when someone unlocks or uses an Apple device
            g_info("  %s Override category from '%s' to '%s', message from %s", local->mac, category_from_int(local->category), category_from_int(remote->category), access_name);
            local->category = remote->category;
        }
        else if (local->category != remote->category) 
        {
            if (local->category == CATEGORY_PHONE && remote->category == CATEGORY_TV)
            {
                // Apple device, originally thought to be phone but is actually a TV
                local->category = CATEGORY_TV;
                g_debug("  %s Changed category from '%s' to '%s', message from %s", local->mac, category_from_int(local->category), category_from_int(remote->category), access_name);
            }
            else
            {
                // messages wearable->phone should be ignored
                g_debug("  %s MAYBE change category from '%s' to '%s', message from %s", local->mac, category_from_int(local->category), category_from_int(remote->category), access_name);
                // TODO: Check any here
            }
        }
    }

    soft_set_u16(&local->appearance, remote->appearance);  // not used ?
    if (remote->try_connect_state >= TRY_CONNECT_COMPLETE) local->try_connect_state = TRY_CONNECT_COMPLETE;  // already connected once
    // TODO: Other fields that we can transfer over

    if (safe)  // i.e. difference between our clock and theirs was zero
    {
        if (remote->latest > local->latest)
        {
            //g_debug("Bumping %s '%s' by %.1fs from %s", local->mac, local->name, difftime(remote->latest, local->latest), access_name);
            local->latest = remote->latest;
        }
    }

}

char* access_point_to_json (struct AccessPoint* a)
{
    char *string = NULL;
    cJSON *j = cJSON_CreateObject();

    // AccessPoint details
    cJSON_AddStringToObject(j, "from", a->client_id);
    cJSON_AddStringToObject(j, "description", a->description);
    cJSON_AddStringToObject(j, "platform", a->platform);
    cJSON_AddNumberToObject(j, "rssi_one_meter", a->rssi_one_meter);
    cJSON_AddNumberToObject(j, "rssi_factor", a->rssi_factor);
    cJSON_AddNumberToObject(j, "people_distance", a->people_distance);
    cJSON_AddNumberToObject(j, "people_closest_count", a->people_closest_count);
    cJSON_AddNumberToObject(j, "people_in_range_count", a->people_in_range_count);

    string = cJSON_PrintUnformatted(j);
    cJSON_Delete(j);
    return string;
}

char* device_to_json (struct AccessPoint* a, struct Device* device)
{
    char *string = NULL;
    cJSON *j = cJSON_CreateObject();

    // AccessPoint details
    cJSON_AddStringToObject(j, "from", a->client_id);
    cJSON_AddStringToObject(j, "description", a->description);
    cJSON_AddStringToObject(j, "platform", a->platform);
    cJSON_AddNumberToObject(j, "rssi_one_meter", a->rssi_one_meter);
    cJSON_AddNumberToObject(j, "rssi_factor", a->rssi_factor);
    cJSON_AddNumberToObject(j, "people_distance", a->people_distance);
    cJSON_AddNumberToObject(j, "people_closest_count", a->people_closest_count);
    cJSON_AddNumberToObject(j, "people_in_range_count", a->people_in_range_count);

    // Device details
    cJSON_AddStringToObject(j, "mac", device->mac);
    cJSON_AddStringToObject(j, "name", device->name);

    char mac_super[18];
    mac_64_to_string(mac_super, 18, device->supersededby);
    cJSON_AddStringToObject(j, "supersededby", mac_super);

    cJSON_AddStringToObject(j, "alias", device->alias);
    cJSON_AddNumberToObject(j, "addressType", device->addressType);
    cJSON_AddStringToObject(j, "category", category_from_int(device->category));
    cJSON_AddNumberToObject(j, "manufacturer", device->manufacturer);
    cJSON_AddBoolToObject(j, "paired", device->paired);
    cJSON_AddBoolToObject(j, "connected", device->connected);
    cJSON_AddBoolToObject(j, "trusted", device->trusted);
    cJSON_AddNumberToObject(j, "deviceclass", device->deviceclass);
    cJSON_AddNumberToObject(j, "appearance", device->appearance);
    cJSON_AddNumberToObject(j, "manufacturer_data_hash", device->manufacturer_data_hash);
    cJSON_AddNumberToObject(j, "service_data_hash", device->service_data_hash);
    cJSON_AddNumberToObject(j, "uuids_length", device->uuids_length);
    cJSON_AddNumberToObject(j, "uuid_hash", device->uuid_hash);
    cJSON_AddNumberToObject(j, "txpower", device->txpower);
    cJSON_AddNumberToObject(j, "last_sent", device->last_sent);
    cJSON_AddNumberToObject(j, "distance", device->distance);
    cJSON_AddNumberToObject(j, "earliest", device->earliest);
    cJSON_AddNumberToObject(j, "latest", device->latest);
    cJSON_AddNumberToObject(j, "count", device->count);
    cJSON_AddNumberToObject(j, "column", device->column);
    cJSON_AddNumberToObject(j, "try_connect_state", device->try_connect_state);
    if (device->is_training_beacon){
        cJSON_AddNumberToObject(j, "training", 1);
    }

    string = cJSON_PrintUnformatted(j);
    cJSON_Delete(j);
    return string;
}

bool device_from_json(const char* json, struct AccessPoint* access_point, struct Device* device)
{
    cJSON *djson = cJSON_Parse(json);

    // ACCESS POINT

    cJSON *fromj = cJSON_GetObjectItemCaseSensitive(djson, "from");
    if (cJSON_IsString(fromj) && (fromj->valuestring != NULL))
    {
        strncpy(access_point->client_id, fromj->valuestring, META_LENGTH);
    }

    cJSON *descriptionj = cJSON_GetObjectItemCaseSensitive(djson, "description");
    if (cJSON_IsString(descriptionj) && (descriptionj->valuestring != NULL))
    {
        strncpy(access_point->description, descriptionj->valuestring, META_LENGTH);
    }

    cJSON *platformj = cJSON_GetObjectItemCaseSensitive(djson, "platform");
    if (cJSON_IsString(platformj) && (platformj->valuestring != NULL))
    {
        strncpy(access_point->platform, platformj->valuestring, META_LENGTH);
    }

    cJSON *rssi_one_meter = cJSON_GetObjectItemCaseSensitive(djson, "rssi_one_meter");
    if (cJSON_IsNumber(rssi_one_meter))
    {
        access_point->rssi_one_meter = rssi_one_meter->valueint;
    }

    cJSON *rssi_factor = cJSON_GetObjectItemCaseSensitive(djson, "rssi_factor");
    if (cJSON_IsNumber(rssi_one_meter))
    {
        access_point->rssi_factor = (float)rssi_factor->valuedouble;
    }

    cJSON *people_distance = cJSON_GetObjectItemCaseSensitive(djson, "people_distance");
    if (cJSON_IsNumber(people_distance))
    {
        access_point->people_distance = (float)people_distance->valuedouble;
    }

    cJSON *people_closest_count = cJSON_GetObjectItemCaseSensitive(djson, "people_closest_count");
    if (cJSON_IsNumber(people_closest_count))
    {
        access_point->people_closest_count = (float)people_closest_count->valuedouble;
    }

    cJSON *people_in_range_count = cJSON_GetObjectItemCaseSensitive(djson, "people_in_range_count");
    if (cJSON_IsNumber(people_in_range_count))
    {
        access_point->people_in_range_count = (float)people_in_range_count->valuedouble;
    }

    // DEVICE

    cJSON *mac = cJSON_GetObjectItemCaseSensitive(djson, "mac");
    if (cJSON_IsString(mac) && (mac->valuestring != NULL))
    {
        strncpy(device->mac, mac->valuestring, 18);
        int64_t mac64 = mac_string_to_int_64(mac->valuestring);
        device->mac64 = mac64;
    }

    cJSON *name = cJSON_GetObjectItemCaseSensitive(djson, "name");
    if (cJSON_IsString(name) && (name->valuestring != NULL))
    {
        strncpy(device->name, name->valuestring, NAME_LENGTH);
    }

    cJSON *supersedes = cJSON_GetObjectItemCaseSensitive(djson, "supersededby");
    if (cJSON_IsString(supersedes))
    {
        // TODO: ulong serialization with cJSON
        device->supersededby = mac_string_to_int_64(supersedes->valuestring);
    }

    cJSON *latest = cJSON_GetObjectItemCaseSensitive(djson, "latest");
    if (cJSON_IsNumber(latest))
    {
        // TODO: Full date time serialization and deserialization
        device->latest = latest->valueint;
    }

    cJSON *distance = cJSON_GetObjectItemCaseSensitive(djson, "distance");
    if (cJSON_IsNumber(distance))
    {
        device->distance = (float)distance->valuedouble;
    }

    cJSON *count = cJSON_GetObjectItemCaseSensitive(djson, "count");
    if (cJSON_IsNumber(count))
    {
        device->count = count->valueint;
    }

    cJSON *earliest = cJSON_GetObjectItemCaseSensitive(djson, "earliest");
    if (cJSON_IsNumber(earliest))
    {
        // TODO: Full date time serialization and deserialization
        device->earliest = earliest->valueint;
    }

    cJSON *training = cJSON_GetObjectItemCaseSensitive(djson, "training");
    device->is_training_beacon = cJSON_IsNumber(training);

    device->category = CATEGORY_UNKNOWN;
    cJSON *category = cJSON_GetObjectItemCaseSensitive(djson, "category");
    if (cJSON_IsString(category) && (category->valuestring != NULL))
    {
        device->category = category_to_int(category->valuestring);
    }

   cJSON_Delete(djson);

   return true;
}

