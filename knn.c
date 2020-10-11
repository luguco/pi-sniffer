/*
    K-Nearest Neighbors Classifier
*/
#include "knn.h"
#include "accesspoints.h"
#include "cJSON.h"
#include "utility.h"

#include <gio/gio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <glib.h>
#include <string.h>
#include <math.h>

// UTILITY METHODS

/*
   Get JSON for a recording as a string, must call free() on string after use
*/
char* recording_to_json (struct recording* r, struct AccessPoint* access_points)
{
    char *string = NULL;
    cJSON *j = cJSON_CreateObject();

    // TODO: Add date time stamp
    cJSON_AddStringToObject(j, "room", r->room_name);

    cJSON *distances = cJSON_AddObjectToObject(j, "distances");
    for (struct AccessPoint* ap = access_points; ap != NULL; ap = ap->next)
    {
        double distance = r->access_point_distances[ap->id];
        if (distance > 0)
        {
            char print_num[18];
            snprintf(print_num, 18, "%.2f", distance);
            cJSON_AddRawToObject(distances, ap->client_id, print_num);
        }
    }

    string = cJSON_PrintUnformatted(j);
    cJSON_Delete(j);
    return string;
}

/*
    Convert JSON lines value back to a recording value using current order of access points
*/
bool json_to_recording(char* buffer, struct AccessPoint* access_points, struct recording* r)
{
    cJSON *json = cJSON_Parse(buffer);
    if (json == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            g_warning("Error reading saved recording before: %s", error_ptr);
        }
        return FALSE;
    }

    cJSON* room_name = cJSON_GetObjectItemCaseSensitive(json, "room");
    if (!cJSON_IsString(room_name))
    {
        cJSON_Delete(json);
        return FALSE;
    }

    //g_debug("Got room %s", room_name->valuestring);

    g_utf8_strncpy(r->room_name, room_name->valuestring, META_LENGTH);
    url_slug(r->room_name);   // strip spaces and any other junk from it

    cJSON* distances = cJSON_GetObjectItemCaseSensitive(json, "distances");

    if (!cJSON_IsObject(distances))
    {
        g_error("Missing distances in saved recording");
        cJSON_Delete(json);
        return FALSE;
    }

    int count = 0;

    for (struct AccessPoint* ap = access_points; ap != NULL; ap = ap->next)
    {
        cJSON* dist = cJSON_GetObjectItem(distances, ap->client_id);
        if (cJSON_IsNumber(dist))
        {
            r->access_point_distances[ap->id] = dist->valuedouble;
            count++;
        }
        else
        {
            r->access_point_distances[ap->id]=0;
        }
    }

    if (count < 1)
    {
        g_warning("No values found in %s", buffer);
        cJSON_Delete(json);
        return FALSE;
    }

    //g_debug("free buffer");
    cJSON_Delete(json);
    return TRUE;
}

// KNN CLASSIFIER

float score (struct recording* recording, double access_points_distance[N_ACCESS_POINTS], struct AccessPoint* access_points)
{
    float sum_delta_squared = 0.0;
    for (struct AccessPoint* ap = access_points; ap != NULL; ap=ap->next)
    {
        float delta = access_points_distance[ap->id] - recording->access_point_distances[ap->id];
        sum_delta_squared += delta*delta;
    }

    return sum_delta_squared;
}


/*
  k_nearest classifier
  Like KNeighborsClassifier(weights='distance')

  parameters
    top_k_list

  NB You must free() the returned string

*/
int k_nearest(struct recording* recordings, double* access_point_distances, struct AccessPoint* access_points, struct top_k* top_result, int top_count)
{
    struct top_k result[TOP_K_N];
    int k = 0;
    for (struct recording* recording = recordings; recording != NULL; recording = recording->next)
    {
        // Insert recording into the list if it's a better match
        float distance = score(recording, access_point_distances, access_points);

        struct top_k current;
        g_utf8_strncpy(current.room_name, recording->room_name, META_LENGTH);
        current.distance = distance;
        current.used = FALSE;

        // Find insertion point
        for (int i = 0; i < TOP_K_N; i++)
        {
            if (i == k)
            {
                // Off the end, but still < k, so add the item here
                k++;
                result[i] = current;
                break;
            }
            else if (result[i].distance > current.distance)
            {
                // Insert at this position, pick up current and move it down
                struct top_k temp = result[i];
                result[i] = current;
                current = temp;
            }
            else
            {
                // keep going
            }
        }
    }

    if (k == 0) {
        top_result->distance = 100000;
        g_utf8_strncpy(top_result->room_name, "unknown", META_LENGTH);
        return 0;
    };

    top_result[0] = result[0];
    float best_score = 0.0;

    float smoothing = 0.1;

    // Find the most common in the top_k weighted by distance
    for (int i = 0; i < k; i++)
    {
        if (result[i].used) continue;

        float best_distance = result[i].distance;
        float score = 1.0 / (smoothing + result[i].distance);
        for (int j = i+1; j < k; j++)
        {
            if (strcmp(result[i].room_name, result[j].room_name) == 0 && result[j].distance > 0)
            {
                score += 1.0 / (smoothing + result[j].distance);
                result[i].used = TRUE;

                if (result[j].distance < best_distance)
                {
                    best_distance = result[i].distance;
                }
            }
        }

        if (score > best_score)
        {
            //g_debug("Replace %s by %s in best score %.2f", top_result[0].room_name, result[i].room_name, score);
            best_score = score;
            top_result[0] = result[i];                        // copy name and distance (for first)
            top_result[0].distance = sqrt(best_distance);     // overwrite with best distance
            // do something with top_count
            (void)top_count;
        }
    }

    return 1;       // just the top result for now
}


// FILE OPERATIONS


bool read_observations_file (const char * dirname, const char* filename, struct AccessPoint* access_points, struct recording** recordings)
{
	g_return_val_if_fail (filename != NULL, FALSE);

    // only match JSONL files
    g_return_val_if_fail(string_ends_with(filename, ".jsonl"), FALSE);

    char fullpath[128];
    g_snprintf(fullpath, sizeof(fullpath), "%s/%s", dirname, filename);

    //gchar *filename = g_strdup_printf("file://%s/.extension", g_get_home_dir());

    GFile *file = g_file_new_for_path (fullpath);

	g_return_val_if_fail (G_IS_FILE (file), FALSE);

    GError **error = NULL;
	GError *error_local = NULL;

	GFileInputStream* is = g_file_read (file, NULL, &error_local);
	if (is == NULL) {
		g_propagate_error (error, error_local);
        g_warning("Could not open file %s: %s", fullpath, error_local->message);
		return FALSE;
	}

	GDataInputStream * input = g_data_input_stream_new (G_INPUT_STREAM (is));

	/* read file line by line */
    int line_count = 0;
	while (TRUE) {
		gchar *line;
        gsize length;
		line = g_data_input_stream_read_line (input, &length, NULL, NULL);
		if (line == NULL)
			break;

        line_count++;

        trim(line);
        if (strlen(line) > 0)
        {
            //g_debug("%s", line);
            struct recording r;
            bool ok = json_to_recording(line, access_points, &r);
            if (ok)
            {
                //g_debug("Got a valid recording for %s", r.room_name);
                // Append it to the front of the recordings list
                // TODO: Remove duplicates
                // NB Only allocate if ok
                struct recording* ralloc = malloc(sizeof(struct recording));
                *ralloc = r;
                ralloc->next = *recordings;
                *recordings = ralloc;
            }
            else
            {
                g_warning("Could not use '%s' in %s (%i)", line, filename, line_count);
            }
        }
        g_free(line);
	}

    // close stream
    g_input_stream_close(G_INPUT_STREAM(is), NULL, &error_local);
    g_object_unref(is);
    g_object_unref(file);
	return TRUE;
}

/*
   free recordings linked list
*/
void free_list(struct recording** head)
{
   struct recording* tmp;

   while (*head != NULL)
    {
       tmp = *head;
       *head = (*head)->next;
       free(tmp);
    }
}

/**
 * Read observations
 * @file: observations file name
 * @error: a %GError to put the error code and message in, or %NULL
 *
 * Reads observations into memory
 *
 * Return value: %TRUE if there were no errors.
 *
 **/
bool read_observations (const char * dirname, struct AccessPoint* access_points, struct recording** recordings)
{

    GDir *dir;
    GError *error;
    const gchar *filename;

    bool ok = TRUE;

    dir = g_dir_open(dirname, 0, &error);

    while ((filename = g_dir_read_name(dir)))
    {
        ok = read_observations_file(dirname, filename, access_points, recordings) && ok;
    }
    g_dir_close(dir);

    return TRUE;
}


// TODO MULTIPLE FILES IN DIRECTORY

/*
    record_observation
*/

bool record (const char* directory, const char* device_name, double access_distances[N_ACCESS_POINTS], struct AccessPoint* access_points, char* location)
{
	GError *error_local = NULL;

    GFile* dir2 = g_file_new_for_path(directory);
    if (g_file_make_directory(dir2, NULL, &error_local))
    {
        g_info("Created recordings directory '%s'", directory);
    }
    g_object_unref(dir2);

    char fullpath[128];
    g_snprintf(fullpath, sizeof(fullpath), "%s/%s.jsonl", directory,  device_name);

    GFile *file = g_file_new_for_path (fullpath);

	g_return_val_if_fail (G_IS_FILE (file), FALSE);

	GFileOutputStream* os = g_file_append_to (file, G_FILE_CREATE_NONE, NULL, &error_local);
	if (os == NULL) {
        g_warning("Could not open %s for writing recordings: %s", fullpath, error_local->message);
		//g_propagate_error (error, error_local);
        g_object_unref(file);
		return FALSE;
	}

	GDataOutputStream * output = g_data_output_stream_new (G_OUTPUT_STREAM (os));

    struct recording r;
    for (int i = 0; i < N_ACCESS_POINTS; i++)
    {
        r.access_point_distances[i] = access_distances[i];
    }
    g_utf8_strncpy(r.room_name, location, META_LENGTH);

    char* buffer = recording_to_json(&r, access_points);
    if (buffer != NULL)
    {
        gssize written =  g_data_output_stream_put_string(output, buffer, NULL, &error_local);
        written +=  g_data_output_stream_put_string(output, "\n", NULL, &error_local);

        free(buffer);
    }

    g_output_stream_close(G_OUTPUT_STREAM (os), NULL, &error_local);      // handles flush
    g_object_unref(output);
    g_object_unref(os);
    g_object_unref(file);

    return TRUE;
}
