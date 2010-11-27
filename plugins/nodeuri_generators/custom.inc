<?php
// $Id$
/**
 * @file
 * Custom node URI generator.
 */

/**
 * Custom URI generator plugin definition.
 */
$plugin = array(
  'title' => t('Custom URI'),
  'description' => t('Generate node URIs from custom patterns using tokens. The Token module must be enabled for this to work.'),
  'callback generate' => 'simplerdf_generate_custom_nodeuri',
  'settings defaults' => array(
    'pattern' => 'node/[nid]',
  ),
  'settings form' => 'simplerdf_nodeuri_custom_settings',
);

/**
 * Generate custom node URI.
 */
function simplerdf_nodeuri_custom_generate($node) {
  if (module_exists('token')) {  // Just to make sure Token is available
    $pattern = simplerdf_get_setting('custom', 'pattern');
    return token_replace($pattern, 'node', $node);
  }
}

/**
 * Settings form.
 */
function simplerdf_nodeuri_custom_settings($form_state) {
  $form['pattern'] = array(
    '#title' => 'Node URI pattern',
    '#type' => 'textfield',
    '#prefix' => base_path(),
    '#default_value' => $form_state['values']['pattern'],
    '#required' => TRUE,
  );
  $form['legend'] = array(
    '#type' => 'markup',
    '#value' => theme('token_help', 'node'),
  );
}
