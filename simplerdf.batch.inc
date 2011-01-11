<?php
/**
 * @file
 * Simple RDF batch architecture. This file is loaded with the main module file.
 */

/**
 * Run batch of tasks for Simple RDF.
 *
 * @param array $batch
 *   A batch array suitable for the Batch API. If anything is provided in the
 *   'operations' key, it will be used in the batch. For each operation, if
 *   instead of an array (as in the Batch API) a string is provided, it will
 *   be searched in the $tasks array as a key and used to build an operation
 *   automatically. If 'operations' is not specified, it will be populated from
 *   the $tasks array. In addition, the batch can include the following keys:
 *   - 'redirect' : (Optional) Target path to redirect to when finished.
 *   - 'process' : (Optional) Whether to process batch.
 *   - 'finished' : (Optional) Finished callback, as in the Batch API. If not
 *     given, a dummy callback will be used instead.
 * @param array $tasks
 *   An array of task arrays. Each task array is keyed by its name and may
 *   contain the following key-value pairs:
 *   - 'count callback' : Callback function computing the initial total number
 *     of steps. This function should return an integer.
 *   - 'perform callback' : Callback function to perform a group of steps in
 *     the task. This function is invoked with the task array and a reference
 *     to the context.
 *   - 'summarize callback' : (Optional) Callback function to summarize each
 *     group of steps. This function is invoked with the task array and a
 *     reference to the context.
 *   When processing a batch, the count callback is first invoked to determine
 *   the number of steps. The perform callback should return the number steps
 *   performed; alternatively, it may increment $context['sandbox']['progress']
 *   by that number. The progress counter is compared with the total count to
 *   determine the progress of the task and whether it has finished.
 */
function simplerdf_batch(array $batch, array $tasks = array()) {
  // Extract options
  $redirect = $batch['redirect'];
  $process = $batch['process'];
  if (!isset($batch['finished'])) {
    $batch['finished'] = 'simplerdf_batch_finished';
  }

  // Prepare batch operations
  $operations = array();
  if (is_array($batch['operations'])) {
    // Transform a task into an operation
    foreach ($batch['operations'] as $operation) {
      if (is_array($operation) && function_exists($operation[0]) && is_array($operation[1])) {
        // Default operation
        $operations[] = $operation;
      }
      elseif (is_string($operation) && is_array($tasks[$operation]) && $task = _simplerdf_batch_validate_task($tasks[$operation])) {
        // Task
        $operations[] = array('simplerdf_batch_operation', array($task));
      }
    }
  }
  elseif (!isset($batch['operations'])) {
    // Populate operations from tasks
    foreach ($tasks as $task) {
      if ($task = _simplerdf_batch_validate_task($task)) {
        $operations[] = array('simplerdf_batch_operation', array($task));
      }
    }
  }
  else {
    // Invalid operations
    return;
  }

  if (empty($operations)) {
    // No operations defined
    if ($process) {
      // Redirect to target
      drupal_goto($redirect);
    }
    else {
      // Terminate directly
      return;
    }
  }

  $batch['operations'] = $operations;

  // Set batch to process
  batch_set($batch);
  if ($process) {
    batch_process($redirect);
  }
}

/**
 * Transform a task into an operation.
 */
function _simplerdf_batch_validate_task($task) {
  // Function checks
  if (function_exists($task['count callback']) && function_exists($task['perform callback'])) {
    // Required callbacks
    if (!isset($task['summarize callback']) || function_exists($task['summarize callback'])) {
      // Optional callback
      return $task;
    }
  }
}

/**
 * Perform a task.
 *
 * @param array $task
 *   Task array as defined in simplerdf_batch().
 * @param array $context
 *   Batch operation context.
 */
function simplerdf_batch_operation($task, &$context) {
  $count_callback = $task['count callback'];
  $perform_callback = $task['perform callback'];

  // Initialize sandbox
  if (empty($context['sandbox'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['count'] = $count_callback();
  }

  // Perform tasks
  if ($context['sandbox']['count'] > 0) {
    $perform_callback($task, $context);
  }

  if ($context['sandbox']['progress'] >= $context['sandbox']['count']) {
    // Mark as complete
    $context['finished'] = 1;
  }

  // Summarize
  if (function_exists($summarize_callback = $task['summarize callback'])) {
    $summarize_callback($task, $context);
  }
}

/**
 * Default batch finished callback.
 */
function simplerdf_batch_finished($success, $results, $operations) {
  if ($success) {
    drupal_set_message(t('The batch has been processed.'));
  }
  else {
    drupal_set_message(t('An error occurred while processing the batch.'), 'error');
  }
}

/**
 * Batch index site objects for Simple RDF.
 *
 * @param array $batch
 *   A batch array as defined for simplerdf_batch(), with one exception: the
 *   'operations' will be ignored and entirely generated from $tasks. The
 *   batch finished callback is also implemented for indexing.
 * @param array $tasks
 *   An array of task arrays. Each task array is as described for
 *   simplerdf_batch(), with the following additions:
 *   - 'summarize callback' : (Optional) As in simplerdf_batch(). If not
 *     specified, 'simplerdf_batch_index_summarize' is provided as default.
 *   - 'object type' : Identifier for the object type being indexed.
 *   - 'type singular' : Singular noun for the object type being indexed.
 *   - 'type plural' : Plural noun for the object type being indexed.
 *
 * @see simplerdf_batch_index_summarize()
 */
function simplerdf_batch_index(array $batch, array $tasks) {
  $redirect = $batch['redirect'];
  $process = $batch['process'];

  // Provide defaults
  $batch += array(
    'title' => t('Indexing site'),
    'finished' => 'simplerdf_batch_index_finished',
  );

  // Scrub operations
  unset($batch['operations']);

  // Prepare tasks
  foreach ($tasks as $name => $task) {
    $tasks[$name] += array(
      'summarize callback' => 'simplerdf_batch_index_summarize',
    );
  }

  simplerdf_batch($batch, $tasks);
}

/**
 * Summarize indexing for a set of objects in a given task.
 */
function simplerdf_batch_index_summarize($task, &$context) {
  static $messages;
  static $type_singular;
  static $type_plural;

  if ($type_singular != $task['type singular'] || $type_plural != $task['type plural']) {
    // Reset cached parameters
    $messages = NULL;
  }

  // Prepare message formats
  if (!isset($messages)) {
    $type_singular = $task['type singular'];
    $type_plural = $task['type plural'];
    $t_args = array('%type' => $type);
    // Singular messages
    if ($type_singular) {
      $messages['summary']['singular'] = 'Indexed @count @type.';
      $messages['progress']['singular'] = 'Indexing @count @type.';
      $t_args['@type'] = $type_singular;
    }
    else {
      $messages['summary']['singular'] = 'Indexed @count %type object.';
      $messages['progress']['singular'] = 'Indexing @count %type object.';
    }
    // Plural messages
    if ($type_plural) {
      $messages['summary']['plural'] = 'Indexed @count @types.';
      $messages['progress']['plural'] = 'Indexing @index of @count @types.';
      $t_args['@types'] = $type_plural;
    }
    else {
      $messages['summary']['plural'] = 'Indexed @count %type objects.';
      $messages['progress']['plural'] = 'Indexing @index of @count %type objects.';
    }
    $messages['t_args'] = $t_args;
  }

  if ($context['sandbox']['progress'] >= $context['sandbox']['count'] && $context['sandbox']['count'] > 0) {
    $summary = format_plural($context['sandbox']['progress'], $messages['summary']['singular'], $messages['summary']['plural'], $messages['t_args']);
    $context['results']['summary'][] = $summary;
  }
  elseif ($context['sandbox']['progress'] < $context['sandbox']['count']) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['count'];
    $context['message'] = format_plural($context['sandbox']['count'], $messages['progress']['singular'], $messages['progress']['plural'], $messages['t_args'] + array('@index' => $context['sandbox']['progress']));
  }
}

/**
 * Finished callback for batch indexing.
 */
function simplerdf_batch_index_finished($success, $results, $operations) {
  if ($success) {
    drupal_set_message(t('The site has been indexed.'));
    if ($results['summary']) {
      // Show all messages
      drupal_set_message(theme('item_list', $results['summary']));
    }
  }
  else {
    // An error occurred.
    $operation = reset($operations);
    $args = $operation[1];
    $task = $args[0];

    $types = simplerdf_get_types();
    $type_title = $types[$task['type']];

    $message = t('An error occurred while indexing %type.', array('%type' => $type_title));
    drupal_set_message($message, 'error');
  }
}
