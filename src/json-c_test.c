
#include <config.h>
#include <stdio.h>
#include <json-c/json.h>

void test_parse_obj_to_string(struct json_object* const obj) {
    json_object_object_foreach(obj, key, val) {
        printf("-- \t%s: %s\n", key, json_object_to_json_string(val));
    }
}

void test_parse_check_type(struct json_object* const obj) {
    json_object_object_foreach(obj, key, val) {
        if (json_object_is_type(val, json_type_null)) {
            printf("-- \tjson_type_null -> \t%s: %s\n",
                    key, json_object_to_json_string(val));
        }
        if (json_object_is_type(val, json_type_boolean)) {
            printf("-- \tjson_type_boolean -> \t%s: %s\n",
                    key, json_object_to_json_string(val));
            printf("-- \t\tvalue: %s\n", json_object_get_boolean(val)? "true": "false");
        }
        if (json_object_is_type(val, json_type_double)) {
            printf("-- \tjson_type_double -> \t%s: %s\n",
                    key, json_object_to_json_string(val));
            printf("-- \t\tvalue: %lf\n", json_object_get_double(val));
        }
        if (json_object_is_type(val, json_type_int)) {
            printf("-- \tjson_type_int -> \t%s: %s\n",
                    key, json_object_to_json_string(val));
            printf("-- \t\tvalue: %d\n", json_object_get_int(val));
        }
        if (json_object_is_type(val, json_type_object)) {
            printf("-- \tjson_type_object -> \t%s: %s\n",
                    key, json_object_to_json_string(val));
            printf(">>> START object \n");
            test_parse_check_type(val);
            printf("<<< END object \n");
        }
        if (json_object_is_type(val, json_type_array)) {
            printf("-- \tjson_type_array -> \t%s: %s\n",
                    key, json_object_to_json_string(val));

            for (int i = 0; i < json_object_array_length(val); ++i) {
                struct json_object *a = json_object_array_get_idx(val, i);
                printf("-- \t\tvalue: [%d]=%s\n", i, json_object_to_json_string(a));
            }
        }
        if (json_object_is_type(val, json_type_string)) {
            printf("-- \tjson_type_object -> \t%s: %s\n",
                    key, json_object_to_json_string(val));
            printf("-- \t\tvalue: %s\n", json_object_get_string(val));
        }
    }
}

int main() {
    puts("\n== json parse test start");

    puts("\n== json parse from srting");
    struct json_object *jobj_from_string = json_tokener_parse("{\"a\":1,\"b\":2,\"c\":3}");
    test_parse_obj_to_string(jobj_from_string);

    puts("\n== json parse from file");
    struct json_object *jobj_from_file = json_object_from_file("./config.json");
    test_parse_obj_to_string(jobj_from_file);

    puts("\n== json parse from file & check type");
    test_parse_check_type(jobj_from_file);

    return 0;
}
