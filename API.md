## API documentation

### `uint32_t uci2_version_numeric(void)`

#### description

Returns the numeric version of the uci2 library.

#### inputs

None

#### outputs

None

#### return value

Numeric version of the uci2 library.

### `const char *uci2_version_string(void)`

#### description

Returns the string version of the uci2 library.

#### inputs

None

#### outputs

None

#### return value

String version of the uci2 library.

### `uci2_error_e uci2_config_parse(const char *config, uci2_ast_t **out)`

#### description

Parses the UCI configuration file and returns the Abstract Syntax Tree (AST) representation of that file.

#### inputs

- `config` - path to the UCI configuration file.

#### outputs

- `out` - AST representation of the UCI configuration file.

#### return value

`UE_NONE, UE_INVALID_ARGUMENT, UE_FILE_NOT_FOUND, UE_FILE_IO, UE_PARSER`

### `uci2_error_e uci2_config_remove(const char *config)`

#### description

Removes the UCI configuration file specified by the path in the input parameter.

#### inputs

- `config` - path to the UCI configuration file.

#### outputs

None

#### return value

`UE_NONE, UE_FILE_NOT_FOUND, UE_FILE_IO`

### `uci2_error_e uci2_ast_create(uci2_ast_t **out)`

#### description

Creates a new AST context with root node only.

#### inputs

None

#### outputs

- `out` - AST context with root node only.

#### return value

`UE_NONE`

### `uci2_error_e uci2_ast_sync(uci2_ast_t *uci2_ast, const char *config)`

#### description

Writes the AST representation of the UCI configuration file to the file specified by the path in the input parameter.

#### inputs

- `uci2_ast` - AST representation of the UCI configuration file.

- `config` - path to the output UCI configuration file.

#### outputs

None

#### return value

`UE_NONE, UE_INVALID_ARGUMENT, UE_NODE_NOT_FOUND, UE_FILE_IO`

### `void uci2_ast_destroy(uci2_ast_t **uci2_ast)`

#### description

Releases the memory allocated by the AST context and sets the `uci2_ast` to `NULL`.

#### inputs

- `uci2_ast` - AST context.

#### outputs

None

#### return value

None

### `uci2_error_e uci2_node_get(uci2_ast_t *uci2_ast, const char *section, const char *option, uci2_node_t **out)`

#### description

Returns the AST node from the AST context specified by the section name and optionally the option name. If `option` parameter is a string then it returns the AST option node, if `option` parameter is `NULL` then it returns the AST section node.

#### inputs

- `uci2_ast` - AST context.

- `section` - UCI section name.

- `option` - UCI option name.

#### outputs

- `out` - AST node.

#### return value

`UE_NONE, UE_INVALID_ARGUMENT, UE_NODE_NOT_FOUND`

### `uci2_error_e uci2_node_section_add(uci2_ast_t *uci2_ast, uci2_node_t *parent, const char *type, const char *name, uci2_node_t **out)`

#### description

Creates a new AST section node specified by the UCI section type and UCI section name and inserts it into the AST context as a child of its parent AST root node.

#### inputs

- `uci2_ast` - AST context.

- `parent` - AST root node which is the parent node of the `out` node.

- `type` - UCI section type.

- `name` - UCI section name.

#### outputs

- `out` - AST section node.

#### return value

`UE_NONE, UE_INVALID_ARGUMENT, UE_NODE_NOT_FOUND, UE_NODE_TYPE_MISMATCH, UE_NODE_DUPLICATE`

### `uci2_error_e uci2_node_option_add(uci2_ast_t *uci2_ast, uci2_node_t *parent, const char *name, const char *value, uci2_node_t **out)`

#### description

Creates a new AST option node specified by the UCI option name and UCI option value and inserts it into the AST context as a child of its parent AST section node.

#### inputs

- `uci2_ast` - AST context.

- `parent` - AST section node which is the parent node of the `out` node.

- `name` - UCI option name.

- `value` - UCI option value.

#### outputs

- `out` - AST option node.

#### return value

`UE_NONE, UE_INVALID_ARGUMENT, UE_NODE_NOT_FOUND, UE_NODE_TYPE_MISMATCH, UE_NODE_DUPLICATE`

### `uci2_error_e uci2_node_list_add(uci2_ast_t *uci2_ast, uci2_node_t *parent, const char *name, uci2_node_t **out)`

#### description

Creates a new AST list node specified by the UCI list name and inserts it into the AST context as a child of its parent AST section node.

#### inputs

- `uci2_ast` - AST context.

- `parent` - AST section node which is the parent node of the `out` node.

- `name` - UCI list name.

#### outputs

- `out` - AST list node.

#### return value

`UE_NONE, UE_INVALID_ARGUMENT, UE_NODE_NOT_FOUND, UE_NODE_TYPE_MISMATCH, UE_NODE_DUPLICATE`

### `uci2_error_e uci2_node_list_element_add(uci2_ast_t *uci2_ast, uci2_node_t *parent, const char *value, uci2_node_t **out)`

#### description

Creates a new AST list element node specified by the UCI section type and UCI list element value and inserts it into the AST context as a child of its parent AST list node.

#### inputs

- `uci2_ast` - AST context.

- `parent` - AST list node which is the parent node of the `out` node.

- `value` - UCI list element value.

#### outputs

- `out` - AST list element node.

#### return value

`UE_NONE, UE_INVALID_ARGUMENT, UE_NODE_NOT_FOUND, UE_NODE_TYPE_MISMATCH`

### `void uci2_node_remove(uci2_node_t *node)`

#### description

Removes the node by marking its parent as `NULL`.

#### inputs

- `node` - AST node to be removed.

#### outputs

None

#### return value

None

### `uci2_error_e uci2_node_iterator_new(uci2_node_t *node, uci2_node_iterator_t **out)`

#### description

Creates a new AST node iterator which will iterate over the children of the AST node specified as the input `node` parameter.

#### inputs

- `node` - AST node whose children will be iterated over with the `out` iterator.

#### outputs

- `out` - AST node iterator.

#### return value

`UE_NONE, UE_INVALID_ARGUMENT, UE_NODE_NOT_FOUND, UE_NODE_TYPE_MISMATCH`

### `void uci2_node_iterator_destroy(uci2_node_iterator_t **node_iterator)`

#### description

Releases the memory allocated by the AST node iterator and sets the `node_iterator` to `NULL`.

#### inputs

- `node_iterator` - AST node iterator to be destroyed.

#### outputs

None

#### return value

None

### `uci2_error_e uci2_node_iterator_next(uci2_node_iterator_t *node_iterator, uci2_node_t **out)`

#### description

Returns the next AST node in the iteration.

#### inputs

- `node_iterator` - AST node iterator.

#### outputs

- `out` - Next AST node in iteration.

#### return value

`UE_NONE, UE_INVALID_ARGUMENT, UE_NODE_NOT_FOUND, UE_ITERATOR_END`

### `uci2_error_e uci2_node_type_get(uci2_node_t *node, uci2_node_type_e *out)`

#### description

Returns the AST node type of the node specified as the input `node` parameter.

#### inputs

- `node` - AST node.

#### outputs

- `out` - AST node type which can be one of the following values: `UNT_ROOT, UNT_SECTION, UNT_OPTION, UNT_LIST, UNT_LIST_ELEMENT`.

#### return value

`UE_NONE, UE_INVALID_ARGUMENT, UE_NODE_NOT_FOUND`

### `uci2_error_e uci2_node_section_type_get(uci2_node_t *node, const char **type)`

#### description

Returns the UCI section type of the node specified as the input `node` parameter.

#### inputs

- `node` - AST section node.

#### outputs

- `out` - UCI section type.

#### return value

`UE_NONE, UE_INVALID_ARGUMENT, UE_NODE_NOT_FOUND, UE_NODE_ATTRIBUTE_MISSING, UE_NODE_TYPE_MISMATCH`

### `uci2_error_e uci2_node_section_type_set(uci2_node_t *node, const char *type)`

#### description

Sets the UCI section type to the node specified as the input `node` parameter.

#### inputs

- `node` - AST section node.

- `type` - UCI section type.

#### outputs

None

#### return value

`UE_NONE, UE_INVALID_ARGUMENT, UE_NODE_NOT_FOUND, UE_NODE_TYPE_MISMATCH`

### `uci2_error_e uci2_node_section_name_get(uci2_node_t *node, const char **name)`

Returns the UCI section name of the node specified as the input `node` parameter.

#### description

#### inputs

- `node` - AST section node.

#### outputs

- `name` - UCI section name.

#### return value

`UE_NONE, UE_INVALID_ARGUMENT, UE_NODE_NOT_FOUND, UE_NODE_ATTRIBUTE_MISSING, UE_NODE_TYPE_MISMATCH`

### `uci2_error_e uci2_node_section_name_set(uci2_node_t *node, const char *name)`

#### description

Sets the UCI section name to the node specified as the input `node` parameter.

#### inputs

- `node` - AST section node.

- `name` - UCI section name.

#### outputs

None

#### return value

`UE_NONE, UE_INVALID_ARGUMENT, UE_NODE_NOT_FOUND, UE_NODE_TYPE_MISMATCH, UE_NODE_DUPLICATE`

### `uci2_error_e uci2_node_option_name_get(uci2_node_t *node, const char **name)`

#### description

Returns the UCI option name of the node specified as the input `node` parameter.

#### inputs

- `node` - AST option node.

#### outputs

- `name` - UCI option name.

#### return value

`UE_NONE, UE_INVALID_ARGUMENT, UE_NODE_NOT_FOUND, UE_NODE_ATTRIBUTE_MISSING, UE_NODE_TYPE_MISMATCH`

### `uci2_error_e uci2_node_option_name_set(uci2_node_t *node, const char *name)`

#### description

Sets the UCI option name to the node specified as the input `node` parameter.

#### inputs

- `node` - AST option node.

- `name` - UCI option name.

#### outputs

None

#### return value

`UE_NONE, UE_INVALID_ARGUMENT, UE_NODE_NOT_FOUND, UE_NODE_TYPE_MISMATCH, UE_NODE_DUPLICATE`

### `uci2_error_e uci2_node_option_value_get(uci2_node_t *node, const char **value)`

#### description

Returns the UCI option value of the node specified as the input `node` parameter.

#### inputs

- `node` - AST option node.

#### outputs

- `value` - UCI option value.

#### return value

`UE_NONE, UE_INVALID_ARGUMENT, UE_NODE_NOT_FOUND, UE_NODE_ATTRIBUTE_MISSING, UE_NODE_TYPE_MISMATCH`

### `uci2_error_e uci2_node_option_value_set(uci2_node_t *node, const char *value)`

#### description

Sets the UCI option value to the node specified as the input `node` parameter.

#### inputs

- `node` - AST option node.

- `value` - UCI option value.

#### outputs

None

#### return value

`UE_NONE, UE_INVALID_ARGUMENT, UE_NODE_NOT_FOUND, UE_NODE_TYPE_MISMATCH`

### `uci2_error_e uci2_node_list_name_get(uci2_node_t *node, const char **name)`

#### description

Returns the UCI list name of the node specified as the input `node` parameter.

#### inputs

- `node` - AST list node.

#### outputs

- `name` - UCI list name.

#### return value

`UE_NONE, UE_INVALID_ARGUMENT, UE_NODE_NOT_FOUND, UE_NODE_ATTRIBUTE_MISSING, UE_NODE_TYPE_MISMATCH`

### `uci2_error_e uci2_node_list_name_set(uci2_node_t *node, const char *name)`

#### description

Sets the UCI list name to the node specified as the input `node` parameter.

#### inputs

- `node` - AST list node.

- `name` - UCI list name.

#### outputs

None

#### return value

`UE_NONE, UE_INVALID_ARGUMENT, UE_NODE_NOT_FOUND, UE_NODE_TYPE_MISMATCH, UE_NODE_DUPLICATE`

### `uci2_error_e uci2_node_list_element_value_get(uci2_node_t *node, const char **value)`

#### description

Returns the UCI list element value of the node specified as the input `node` parameter.

#### inputs

- `node` - AST list element node.

#### outputs

- `value` - UCI list element value.

#### return value

`UE_NONE, UE_INVALID_ARGUMENT, UE_NODE_NOT_FOUND, UE_NODE_ATTRIBUTE_MISSING, UE_NODE_TYPE_MISMATCH`

### `uci2_error_e uci2_node_list_element_value_set(uci2_node_t *node, const char *value)`

#### description

Sets the UCI list element value to the node specified as the input `node` parameter.

#### inputs

- `node` - AST list element node.

- `value` - UCI list element value.

#### outputs

None

#### return value

`UE_NONE, UE_INVALID_ARGUMENT, UE_NODE_NOT_FOUND, UE_NODE_TYPE_MISMATCH`

### `uci2_error_e uci2_string_to_boolean(const char *string_value, bool *out)`

#### description

Returns the boolean value of the UCI string representation of the boolean value specified as the input `string_value` parameter.

#### inputs

- `string_value` - UCI string representation of a boolean value.

#### outputs

- `out` - Boolean value.

#### return value

`UE_NONE, UE_INVALID_ARGUMENT`

### `const char *uci2_error_description_get(uci2_error_e error)`

#### description

Returns the string description of the uci2 library error.

#### inputs

- `error` - uci2 library error which can be one of the following values: `UE_NONE, UE_INVALID_ARGUMENT, UE_FILE_NOT_FOUND, UE_FILE_IO, UE_PARSER, UE_NODE_NOT_FOUND, UE_NODE_TYPE_MISMATCH, UE_NODE_ATTRIBUTE_MISSING, UE_NODE_DUPLICATE, UE_ITERATOR_END`.

#### outputs

None

#### return value

String description of the uci2 library error.

## Note about handling unnamed sections

When parsing the UCI configuration file the parser names each unnamed section as `@<section_type>[<index>]` where `<index>` represents the index of that unnamed section which starts at `0` and increments on each unnamed section with the same section type.

For example, if there is a UCI configuration file with the following content:

```
config redirect
    option src            wan
    option src_dport      80
    option dest           lan
    option dest_ip        192.168.16.235
    option dest_port      80
    option proto          tcp

config redirect
    option src            wan
    option src_dport      22001
    option dest           lan
    option dest_port      22
    option proto          tcp

config redirect
    option src            lan
    option src_ip         192.168.45.2
    option src_mac        00:11:22:33:44:55
    option src_port       1024
    option src_dport      80
    option dest_ip        194.25.2.129
    option dest_port      120
    option proto          tcp
```

the sections would be named `@redirect[0]`, `@redirect[1]` and `@redirect[2]`.

If section `@redirect[1]` is removed by calling the API call `uci2_node_remove`, the remaining sections in the AST will be sections `@redirect[0]` and `@redirect[2]` as long as the AST is not destroyed.

If the AST with removed section `@redirect[1]` is saved to a file that file would have the following content:

```
config redirect
    option src            wan
    option src_dport      80
    option dest           lan
    option dest_ip        192.168.16.235
    option dest_port      80
    option proto          tcp

config redirect
    option src            lan
    option src_ip         192.168.45.2
    option src_mac        00:11:22:33:44:55
    option src_port       1024
    option src_dport      80
    option dest_ip        194.25.2.129
    option dest_port      120
    option proto          tcp
```

If the file is parsed again by calling the API call `uci2_config_parse` the sections would be named `@redirect[0]` and `@redirect[1]` because that would be the new AST context based on the current content of the UCI configuration file.