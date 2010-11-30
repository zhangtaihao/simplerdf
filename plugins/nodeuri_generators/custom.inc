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
  'callback generate' => 'simplerdf_nodeuri_custom_generate',
  'settings defaults' => array(
    'pattern' => 'node/[nid]',
  ),
  'settings form' => 'simplerdf_nodeuri_custom_settings',
  'weight' => 10,
);

/**
 * Generate custom node URI.
 */
function simplerdf_nodeuri_custom_generate($node, $settings) {
  if (module_exists('token')) {  // Just to make sure Token is available
    $pattern = $settings['pattern'];
    return url(token_replace($pattern, 'node', $node), array('absolute' => TRUE, 'alias' => TRUE));
  }
}

/**
 * Settings form.
 */
function simplerdf_nodeuri_custom_settings($form_state) {
  $form['pattern'] = array(
    '#title' => 'Node URI pattern',
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
    '#value' => theme('token_help', 'node'),
  );

  return $form;
}
