<?php
// $Id$
/**
 * @file
 * SPARQL store/endpoint configuration and functions.
 */

/**
 * Return ARC2 store configuration.
 *
 * @return array
 *   ARC2 store configuration array.
 */
function simplerdf_sparql_store_config() {
  /* MySQL and endpoint configuration */
  global $db_url;
  if (is_array($db_url)) {
    // Multi-connection
    $url = parse_url($db_url['default']);
  }
  else {
    // Regular connection string
    $url = parse_url($db_url);
  }
  if ($url['scheme'] && in_array($url['scheme'], array('mysql', 'mysqli')) && $write_key = variable_get('simplerdf_sparql_arc2_write_key', NULL)) {
    $config = array(
      /* db */
      'db_name' => substr(urldecode($url['path']), 1),
      'db_user' => urldecode($url['user']),
      'db_pwd' => isset($url['pass']) ? urldecode($url['pass']) : '',
      /* store */
      'store_name' => db_prefix_tables('{simplerdf_sparql_arc2}'),

      /* endpoint */
      'endpoint_features' => array(
        'select', 'construct', 'ask', 'describe',
        'load', 'insert', 'delete',
        'dump' /* dump is a special command for streaming SPOG export */
      ),
      'endpoint_timeout' => 60, /* not implemented in ARC2 preview */
      'endpoint_read_key' => variable_get('simplerdf_sparql_arc2_read_key', ''), /* optional */
      'endpoint_write_key' => variable_get('simplerdf_sparql_arc2_write_key', ''), /* optional */
      'endpoint_max_limit' => 500, /* optional */
    );
    return $config;
  }
}

/**
 * Get an ARC2 store object.
 *
 * @return ARC2_Store
 *   Initialized ARC2 store, or NULL if cannot initialize.
 */
function simplerdf_sparql_store_get_store() {
  return _simplerdf_sparql_store_get('store');
}

/**
 * Get an ARC2 endpoint object.
 *
 * @return ARC2_StoreEndpoint
 *   Initialized ARC2 SPARQL endpoint, or NULL if cannot initialize.
 */
function simplerdf_sparql_store_get_endpoint() {
  return _simplerdf_sparql_store_get('endpoint');
}

/**
 * Get an ARC2 store/endpoint instance.
 */
function _simplerdf_sparql_store_get($type) {
  static $store;
  static $endpoint;
  if ($type != 'store' && $type != 'endpoint') {
    return;
  }

  if (!isset($$type)) {
    if ($$type = _simplerdf_sparql_store_make($type)) {
      // Initialize
      if (!$$type->isSetUp()) {
        $$type->setUp();
      }
    }
    else {
      $$type = FALSE;
    }
  }
  return $$type ? $$type : NULL;
}

/**
 * Construct an ARC2 store/endpoint object.
 */
function _simplerdf_sparql_store_make($type) {
  static $init;
  if (!$init) {
    // Initialize ARC2
    $arc_path = function_exists('libraries_get_path') ? libraries_get_path('arc') : 'sites/all/libraries/arc';
    if (file_exists($arc_file = $arc_path . '/ARC2.php')) {
      require_once $arc_file;
      $init = TRUE;
    }
  }
  if (class_exists('ARC2') && $config = simplerdf_sparql_store_config()) {
    $method = $type == 'endpoint' ? 'getStoreEndpoint' : 'getStore';
    return ARC2::$method($config);
  }
}
