/* input.c - YAML input parser for WEEKS calculator
 * 
 * Reads conductor configuration from YAML files
 * Requires: libyaml (libyaml-dev package on Ubuntu/Debian)
 * 
 * YAML format:
 * frequency: 30e6
 * conductors:
 *   - name: line0
 *     w: 2800e-6
 *     h: 2.0e-6
 *     ...
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <yaml.h>
#include "weeks.h"
#include "mf.h"

#define MAX_CONDUCTORS 10

/* Global frequency variable */
double global_frequency = 30e6;  /* Default 30 MHz */

/* Helper function to get scalar value from YAML */
static char* get_scalar_value(yaml_event_t *event) {
    if (event->type != YAML_SCALAR_EVENT) {
        return NULL;
    }
    return strdup((char*)event->data.scalar.value);
}

/* Parse a conductor from YAML */
static int parse_conductor(yaml_parser_t *parser, conductor *c) {
    yaml_event_t event;
    char *key = NULL;
    int in_mapping = 1;
    
    /* Set defaults */
    c->w = 0.0;
    c->h = 0.0;
    c->x = 0.0;
    c->y = 0.0;
    c->b = 0.5;
    c->nw = 10;
    c->nh = 10;
    c->er = 1.0;
    c->substrate_h = 0.0;
    c->tan_delta = 0.0;
    
    while (in_mapping) {
        if (!yaml_parser_parse(parser, &event)) {
            fprintf(stderr, "YAML parse error\n");
            return 0;
        }
        
        switch (event.type) {
            case YAML_MAPPING_END_EVENT:
                in_mapping = 0;
                break;
                
            case YAML_SCALAR_EVENT:
                if (key == NULL) {
                    /* This is a key */
                    key = get_scalar_value(&event);
                } else {
                    /* This is a value */
                    char *value = get_scalar_value(&event);
                    
                    if (strcmp(key, "name") == 0) {
                        /* Store name if needed */
                    } else if (strcmp(key, "w") == 0) {
                        c->w = atof(value);
                    } else if (strcmp(key, "h") == 0) {
                        c->h = atof(value);
                    } else if (strcmp(key, "x") == 0) {
                        c->x = atof(value);
                    } else if (strcmp(key, "y") == 0) {
                        c->y = atof(value);
                    } else if (strcmp(key, "b") == 0) {
                        c->b = atof(value);
                    } else if (strcmp(key, "nw") == 0) {
                        c->nw = atoi(value);
                    } else if (strcmp(key, "nh") == 0) {
                        c->nh = atoi(value);
                    } else if (strcmp(key, "er") == 0) {
                        c->er = atof(value);
                    } else if (strcmp(key, "substrate_h") == 0) {
                        c->substrate_h = atof(value);
                    } else if (strcmp(key, "tan_delta") == 0) {
                        c->tan_delta = atof(value);
                    }
                    
                    free(value);
                    free(key);
                    key = NULL;
                }
                break;
                
            default:
                break;
        }
        
        yaml_event_delete(&event);
    }
    
    c->n = c->nw * c->nh;
    
    fprintf(stderr, "\n  Conductor: w=%.2e, h=%.2e, er=%.2f, substrate_h=%.2e, tan_delta=%.4f",
            c->w, c->h, c->er, c->substrate_h, c->tan_delta);
    
    return 1;
}

conductor *getinput(FILE *fp, int *n) {
    yaml_parser_t parser;
    yaml_event_t event;
    conductor *conductors;
    int conductor_count = 0;
    int in_conductors_sequence = 0;
    char *key = NULL;
    
    conductors = (conductor *)Malloc(sizeof(conductor) * MAX_CONDUCTORS);
    
    /* Initialize parser */
    if (!yaml_parser_initialize(&parser)) {
        fprintf(stderr, "Failed to initialize YAML parser\n");
        return NULL;
    }
    
    yaml_parser_set_input_file(&parser, fp);
    
    fprintf(stderr, "\nParsing YAML input file...");
    
    /* Parse YAML */
    int done = 0;
    while (!done) {
        if (!yaml_parser_parse(&parser, &event)) {
            fprintf(stderr, "YAML parse error at line %lu\n", parser.problem_mark.line);
            yaml_parser_delete(&parser);
            return NULL;
        }
        
        switch (event.type) {
            case YAML_STREAM_END_EVENT:
                done = 1;
                break;
                
            case YAML_SCALAR_EVENT:
                if (!in_conductors_sequence) {
                    if (key == NULL) {
                        key = get_scalar_value(&event);
                    } else {
                        /* Top-level key-value pair */
                        char *value = get_scalar_value(&event);
                        
                        if (strcmp(key, "frequency") == 0) {
                            global_frequency = atof(value);
                            fprintf(stderr, "\nFrequency: %.2e Hz (%.2f MHz)",
                                    global_frequency, global_frequency/1e6);
                        }
                        
                        free(value);
                        free(key);
                        key = NULL;
                    }
                }
                break;
                
            case YAML_SEQUENCE_START_EVENT:
                if (key && strcmp(key, "conductors") == 0) {
                    in_conductors_sequence = 1;
                    free(key);
                    key = NULL;
                }
                break;
                
            case YAML_SEQUENCE_END_EVENT:
                in_conductors_sequence = 0;
                break;
                
            case YAML_MAPPING_START_EVENT:
                if (in_conductors_sequence) {
                    if (conductor_count < MAX_CONDUCTORS) {
                        if (parse_conductor(&parser, &conductors[conductor_count])) {
                            conductor_count++;
                        }
                    }
                }
                break;
                
            default:
                break;
        }
        
        yaml_event_delete(&event);
    }
    
    yaml_parser_delete(&parser);
    
    *n = conductor_count;
    
    fprintf(stderr, "\n\nTotal conductors loaded: %d\n", conductor_count);
    
    return conductors;
}
