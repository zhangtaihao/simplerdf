<?php
// $Id$
/**
 * @file
 * Custom node URI format.
 */

/**
 * Custom URI generator plugin definition.
 */
$plugin = array(
  'title' => t('Custom URI'),
  'description' => t('Format URIs from custom patterns using tokens. The Token module must be enabled for this to work.'),
  'internal' => TRUE,
  'callback format' => 'simplerdf_uri_token_format',
  'settings defaults' => array(
    'pattern' => '',
  ),
  'settings form' => 'simplerdf_uri_token_custom_settings',
);

/**
 * Format custom node URI.
 */
function simplerdf_uri_token_format($type, $id, $object, array $config = array()) {
  if (function_exists('token_replace')) {  // Just to make sure Token is available
    $pattern = $config['settings']['pattern'];
    if (!isset($object)) {
      // Attempt to load object
      $object = simplerdf_load($type, $id);
    }
    if (is_object($object)) {
      // Translate to token types
      if ($type == 'term') {
        $token_type = 'taxonomy';
      }
      else {
        $token_type = $type;
      }
      return token_replace($pattern, $token_type, $object);
    }
  }
}

/**
 * Settings form.
 */
function simplerdf_uri_token_custom_settings($form_state, $type) {
  // Translate to token types
  if ($type == 'term') {
    $token_type = 'taxonomy';
  }
  else {
    $token_type = $type;
  }

  $form['pattern'] = array(
    '#title' => 'Pattern',
    '#type' => 'textfield',
    '#field_prefix' => url(NULL, array('absolute' => TRUE)),
    '#default_value' => $form_state['values']['pattern'],
    '#required' => TRUE,
  );
  $form['token_help'] = array(
    '#title' => t('Replacement patterns'),
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#weight' => 10,
  );
  $form['token_help']['help'] = array(
    '#type' => 'markup',
    '#value' => theme('token_help', $token_type),
  );

  return $form;
}
