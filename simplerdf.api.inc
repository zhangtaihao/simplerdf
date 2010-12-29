<?php
// $Id$
/**
 * @file
 * Simple RDF public API. This file is loaded with the main module file.
 */

/************************************************
 * General info.
 ************************************************/

/**
 * Get a list of mappable object types.
 */
function simplerdf_get_types() {
  static $types;
  if (!isset($types)) {
    $info = _simplerdf_type_info();
    $types = $info['types'];
  }
  return $types;
}

/**
 * Build object type info.
 */
function _simplerdf_type_info() {
  static $info;
  if (!isset($info)) {
    $info = array('types' => array(), 'map' => array());
    foreach (module_implements('simplerdf_types') as $module) {
      if (is_array($result = module_invoke($module, 'simplerdf_types'))) {
        // Record type title
        $info['types'] = array_merge($info['types'], $result);
        // Map each provided type to module
        foreach ($result as $type => $_) {
          $info['map'][$type] = $module;
        }
      }
    }
  }
  return $info;
}

/**
 * Load an object by its type and ID.
 *
 * @param string $type
 *   Object type, e.g. node.
 * @param int $id
 *   Object ID.
 * @param boolean $reset
 *   (Optional) Wherever applicable, whether or not to reset working cache.
 *
 * @return
 *   Loaded object, or NULL if loading failed.
 */
function simplerdf_load($type, $id, $reset = FALSE) {
  $info = _simplerdf_type_info();
  if ($type && $id && is_numeric($id) && ($module = $info['map'][$type]) && function_exists($callback = $module . '_simplerdf_' . $type . '_load')) {
    $object = $callback($id, $reset);
    if (is_object($object)) {
      return $object;
    }
  }
}


/************************************************
 * Mapping field.
 ************************************************/

/**
 * Map a node field to an RDF property and generate RDF triples.
 *
 * @param string $type
 *   Object type, e.g. node.
 * @param $object
 *   Node object.
 * @param string $uri
 *   Object URI as RDF statement subject.
 * @param array $properties
 *   An array of RDF properties as predicates.
 * @param array $field
 *   Mapping field.
 *
 * @return array
 *   RDF triples in the format specified by the ARC2 library, with the
 *   exception of the use of the variable type for any part of the triple.
 *
 * @see http://arc.semsol.org/docs/v2/structures
 */
function simplerdf_map_field($type, $object, $uri, array $properties, array $field) {
  ctools_include('plugins');

  // Map field
  $plugin = simplerdf_get_plugins('mappers', $field['mapper']);
  $triples = array();
  foreach ($properties as $property) {
    $result = _simplerdf_invoke($plugin, 'callback map', $type, $object, $uri, $property, $field);
    if (is_array($result) && !empty($result)) {
      $triples = array_merge($triples, $result);
    }
  }

  return $triples;
}

/**
 * Get a list of fields to map.
 *
 * @param string $type
 *   Object type, e.g. node.
 * @param string $classifier
 *   (Optional) Classifier, such as node type for node.
 *
 * @return array
 *   Fields available for mapping to properties, keyed by mapper.
 */
function simplerdf_get_fields($type, $classifier = NULL) {
  return _simplerdf_build_fields($type, $classifier);
}

/**
 * Lazily build mapper field information.
 *
 * @param string $type
 *   Object type, e.g. node.
 * @param string $classifier
 *   (Optional) Object classifier, such as node type for node.
 * @param boolean $reset
 *   If TRUE, resets the fields cache.
 *
 * @return array
 *   An array of fields keyed by name (NULL if invalid type).
 */
function _simplerdf_build_fields($type, $classifier = NULL, $reset = FALSE) {
  static $fields = array();

  // Internal use only; reset static cache
  if (!isset($type) && !isset($classifier)) {
    if ($reset) {
      $fields = array();
    }
    return;
  }

  if ($reset || !isset($fields[$type][$classifier])) {
    // Fetch from cache
    $cid = "fields:$type:$classifier";
    if (!$reset && $cache = cache_get($cid, 'cache_simplerdf')) {
      $fields[$type][$classifier] = $cache->data;
    }
    // Rebuild cache
    elseif ($type) {
      $plugins = simplerdf_get_plugins('mappers');

      // Lookup field providers
      $providers = array();
      foreach ($plugins as $plugin_name => $plugin) {
        if (!$plugin['object types'] || in_array($type, $plugin['object types'])) {
          $providers[] = $plugin_name;
        }
      }

      // Build field data
      $fields[$type][$classifier] = array();
      foreach ($providers as $plugin_name) {
        $plugin = $plugins[$plugin_name];
        // Fetch fields
        foreach (_simplerdf_invoke($plugin, 'callback fields', $type, $classifier) as $name => $field) {
          $field['mapper'] = $plugin_name;
          $fields[$type][$classifier][$name] = array('name' => $name) + $field;
        }
      }
      cache_set($cid, $fields[$type][$classifier], 'cache_simplerdf');
    }
  }
  return $fields[$type][$classifier];
}

/**
 * Get a list of content fields to map.
 *
 * @param string $content_type
 *   Content type name.
 *
 * @return array
 *   Fields available for mapping to properties, keyed by content mapper.
 */
function simplerdf_get_content_fields($content_type) {
  return _simplerdf_build_content_fields($content_type);
}

/**
 * Lazily build content mapper field information.
 *
 * @param string $content_type
 *   Content type name.
 * @param boolean $reset
 *   If TRUE, resets the fields cache.
 *
 * @return array
 *   An array of fields keyed by name (NULL if invalid type).
 */
function _simplerdf_build_content_fields($content_type, $reset = FALSE) {
  static $fields = array();

  // Internal use only; reset static cache
  if (!isset($content_type)) {
    if ($reset) {
      $fields = array();
    }
    return;
  }

  if ($reset || !isset($fields[$content_type])) {
    $cid = "content_fields:$content_type";
    // Fetch from cache
    if (!$reset && $cache_fields = cache_get($cid, 'cache_simplerdf')) {
      $fields[$content_type] = $cache_fields->data;
    }
    // Rebuild cache
    else {
      // Prepare content types
      $content_type_info = content_types($content_type);
      $content_fields = $content_type_info['fields'];
      if (!empty($content_fields)) {
        // Fetch and sort plugins
        $plugins = simplerdf_get_plugins('content_mappers');

        // Build field data
        $fields[$content_type] = array();
        foreach ($content_fields as $content_field) {
          $field_name = $content_field['field_name'];
          // Lookup field providers
          $providers = array();
          foreach ($plugins as $plugin_name => $plugin) {
            if (in_array($content_field['module'], $plugin['field modules']) || in_array($content_field['type'], $plugin['field types'])) {
              $providers[] = $plugin_name;
            }
          }
          if (empty($providers)) {
            // Specify default mapper
            $providers[] = 'default';
          }

          // Forward field to plugins providing its type
          foreach ($providers as $plugin_name) {
            $plugin = $plugins[$plugin_name];
            foreach (_simplerdf_invoke($plugin, 'callback fields', $content_field) as $name => $field) {
              $field['mapper'] = 'content';
              $field['field type'] = $content_field['type'];
              $field['field module'] = $content_field['module'];
              $field['content field'] = $field_name;
              $field['content mapper'] = $plugin_name;
              $fields[$content_type][$name] = array('name' => $name) + $field;
            }
          }
        }
        cache_set($cid, $fields[$content_type], 'cache_simplerdf');
      }
    }
  }
  return isset($content_type) ? $fields[$content_type] : $fields;
}


/************************************************
 * RDF generation/processing.
 ************************************************/

/**
 * Get RDF triples for an object.
 *
 * @param string $type
 *   Object type, e.g. node.
 * @param int $id
 *   Object ID.
 * @param $object
 *   Loaded object.
 *
 * @return array
 *   Namespace-prefixed RDF triples.
 */
function simplerdf_get_triples($type, $id, $object) {
  if ($type && $id && is_object($object)) {
    // Classify object
    $classifiers = module_invoke_all('simplerdf_classify', $type, $object);
    if (!in_array(NULL, $classifiers, TRUE)) {
      $classifiers = array_merge(array(NULL), $classifiers);
    }

    // Generate URI
    $uri = simplerdf_get_uri($type, $id, $object);

    // Build triples for each classifier
    $triples = array();
    foreach ($classifiers as $classifier) {
      if (is_array($result = simplerdf_build_triples($type, $classifier, $id, $object, $uri))) {
        $triples = array_merge($triples, $result);
      }
    }

    // Give modules a chance to perform some hardcore postprocessing
    drupal_alter('simplerdf_triples', $triples, $type, $id, $object, $uri);

    return $triples;
  }
}

/**
 * Build RDF triples for an object of a specific classification.
 *
 * @param string $type
 *   Object type, e.g. node.
 * @param string $classifier
 *   Object classifier; can be NULL.
 * @param int $id
 *   Object ID.
 * @param $object
 *   Loaded object.
 * @param string $uri
 *   Object URI.
 *
 * @return array
 *   Namespace-prefixed RDF triples.
 */
function simplerdf_build_triples($type, $classifier, $id, $object, $uri) {
  if ($type && $id && is_object($object)) {
    $fields = simplerdf_get_fields($type, $classifier);
    $mappings = simplerdf_get_mappings($type, $classifier);

    // Map each field
    $triples = array();
    foreach ($fields as $field_name => $field) {
      if (isset($mappings[$field_name])) {
        $triples = array_merge($triples, simplerdf_map_field($type, $object, $uri, $mappings[$field_name], $field));
      }
    }

    return $triples;
  }
}

/**
 * Process raw RDF triples for output to ARC2.
 *
 * @param array $triples
 *   Array of raw RDF triples.
 *
 * @return array
 *   Triples suitable for ARC2.
 */
function simplerdf_expand_triples($triples) {
  // Expand known namespace prefixes
  foreach ($triples as &$triple) {
    // Subject
    if ($triple['s_type'] == 'uri') {
      $value = _simplerdf_expand_ns_prefix($triple['s']);
      if (isset($value)) {
        $triple['s'] = $value;
      }
    }
    // Predicate
    $value = _simplerdf_expand_ns_prefix($triple['p']);
    if (isset($value)) {
      $triple['p'] = $value;
    }
    // Object
    if ($triple['o_type'] == 'uri') {
      $value = _simplerdf_expand_ns_prefix($triple['o']);
      if (isset($value)) {
        $triple['o'] = $value;
      }
    }
    if ($triple['o_datatype']) {
      $datatype = _simplerdf_expand_ns_prefix($triple['o_datatype']);
      if (isset($datatype)) {
        $triple['o_datatype'] = $datatype;
      }
    }
  }
  unset($triple);

  return $triples;
}

/**
 * Expand a prefixed value to its full URI
 */
function _simplerdf_expand_ns_prefix($value) {
  if (($prefix = _simplerdf_extract_ns_prefix($value)) && ($ns = simplerdf_get_namespaces($prefix))) {
    return $ns . substr($value, strlen($prefix) + 1);
  }
}

/**
 * Extract a possible namespace prefix.
 */
function _simplerdf_extract_ns_prefix($value) {
  static $protocols = array('http', 'https', 'ftp', 'news', 'nntp', 'telnet', 'mailto', 'irc', 'ssh', 'sftp', 'webcal', 'rtsp');

  // Match only against possible protocol/prefix
  if ($segment_length = strcspn($value, ':')) {
    $segment = substr($value, 0, $segment_length);
    // Only return if not a protocol
    if (!in_array($segment, $protocols)) {
      return $segment;
    }
  }
}


/************************************************
 * Object URI.
 ************************************************/

/**
 * Get URI of an object by its ID.
 *
 * @param string $type
 *   Object type, e.g. node.
 * @param int $id
 *   Object ID.
 * @param $object
 *   (Optional) Loaded object.
 *
 * @return string
 *   Object URI, or NULL if none available.
 */
function simplerdf_get_uri($type, $id, $object = NULL) {
  if ($type && $id && (!isset($object) || is_object($object))) {
    $cid = "$type:$id:uri";
    if ($cache = cache_get($cid, 'cache_simplerdf')) {
      // Fetch cached URI
      return $cache->data;
    }
    elseif ($uri = simplerdf_format_uri($type, $id, $object)) {
      // URI generated
      cache_set($cid, $uri, 'cache_simplerdf');
      return $uri;
    }
  }
}

/**
 * Format object URI.
 *
 * @param string $type
 *   Object type, e.g. node.
 * @param int $id
 *   Object ID.
 * @param $object
 *   (Optional) Loaded object.
 * @param string $format
 *   (Optional) URI format to use. If not given, the active format is used.
 *
 * @return string
 *   Object URI, or NULL if none generated.
 */
function simplerdf_format_uri($type, $id, $object = NULL, $format = NULL) {
  static $plugins = array();
  if ($type && $id && (!isset($object) || is_object($object))) {
    if ($format) {
      $plugin = simplerdf_get_plugins('uri_formats', $format);
    }
    // Get active URI format for object type
    elseif ($plugins[$type]) {
      // Use cached plugin
      $plugin = $plugins[$type];
    }
    elseif ($plugin = simplerdf_get_uri_format_plugin($type)) {
      // Cache plugin for type
      $plugins[$type] = $plugin;
    }
    // Format using active URI format
    if ($plugin) {
      // Prepare format settings
      $format = $plugin['name'];
      $all_settings = (array) variable_get('simplerdf_uri_settings', array());
      $config = (array) $all_settings[$type][$format];
      if (!isset($config['settings'])) {
        $config['settings'] = array();
      }
      $config['settings'] += $plugin['settings defaults'];

      // Generate
      if ($uri = _simplerdf_invoke($plugin, 'callback format', $type, $id, $object, $config)) {
        if ($plugin['internal']) {
          // Transform path to absolute path
          $url_options = array('absolute' => TRUE);
          if (!$config['alias']) {
            // Don't look up alias
            $url_options['alias'] = TRUE;
          }
          $uri = url($uri, $url_options);
        }
        return $uri;
      }
    }
  }
}

/**
 * Generate default object URI.
 *
 * @param string $type
 *   Type of object, e.g. node.
 * @param int $id
 *   Object ID.
 *
 * @return string
 *   Basic path made by joining $type and $id with a slash.
 */
function simplerdf_get_default_uri($type, $id) {
  return "$type/$id";
}
