<?php
/**
 * @file
 * Implements the experiment content type of node. 
 */

/**
 * Implements hook_node_access().
 * 
 * @param $node
 * Tee node on which the operation is performed, or, if it does not yet exist, the 
 * type of node to be created.
 *
 * @param $op
 * The operation to be performed. 
 * 
 * @param $account
 * A user object representing the user for whom the operation is to be performed. 
 *
 * @return 
 * If the permission for the specified operation is not set then return FALSE. If the
 * permissions is set then return NULL as this allwos other modules to disable
 * access. The only exception is when the $op == 'create'. We will always
 * return TRUE if the permission is set. 
 * 
 * @ingroup tripal_experiment
 * 
 */
function chado_experiment_node_access($node, $op, $account) {
  $node_type = $node;
  if (is_object($node)) {
    $node_type = $node->type;
  }

  if ($node_type == 'chado_experiment') {
    if ($op == 'create') {
      if (!user_access('create chado_experiment content', $account)) {
        return NODE_ACCESS_DENY;
      }
      return NODE_ACCESS_ALLOW;
    }
    if ($op == 'update') {
      if (!user_access('edit chado_experiment content', $account)) {
        return NODE_ACCESS_DENY;
      }
    }
    if ($op == 'delete') {
      if (!user_access('delete chado_experiment content', $account)) {
        return NODE_ACCESS_DENY;
      }
    }
    if ($op == 'view') {
      if (!user_access('access chado_experiment content', $account)) {
        return NODE_ACCESS_DENY;
      }
    }
    return NODE_ACCESS_IGNORE;
  }
}

/**
 * Implements hook_form()
 * 
 * @ingroup tripal_experiment
 */
function chado_experiment_form($node, &$form_state) {
  $form = array();

  // Not sure what this is for. Is this line necessary? 
  $form['#attribute']['enctype'] = 'multipart/form-data';
 
  // === get default value for the form ===
  if (property_exists($node, 'experiment')) {   
    $experiment = $node->experiment;

    // Fill out the form. *** change title in protocl
    $experiment_name = property_exists($node, 'experiment_name') ? property_exists($node, 'experiment_name') : $experiment->name;
    $description = property_exists($node, 'description') ? property_exists($node, 'description') : $experiment->description;

    // The conditional here is required because this field is chosen by an optional select.
    if ($experiment->project_id != NULL) {
      $bioproject_id = property_exists($node, 'bioproject_id') ? property_exists($node, 'bioproject_id') : $experiment->project_id->project_id;
    }
    else {
      drupal_set_message(t('Unable to add experiment.'), 'error');
      $bioproject_id = NULL;
    }
 
    if ($experiment->biomaterial_id != NULL) { 
      $biosample_id = property_exists($node, 'biosample_id') ? property_exists($node, 'biosample_id') : $experiment->biomaterial_id->biomaterial_id;
    } else {
      drupal_set_message(t('Unable to add experiment.'), 'error');
    }

    $rank = property_exists($node, 'rank') ? property_exists($node, 'rank') : $experiment->rank;

    // Set the bioproject id. 
    $form['experiment_id'] = array(
      '#type' => 'value',
      '#value' => $experiment->experiment_id,
    );
    $experiment_id = $experiment->experiment_id;
  }
  else {
    $experiment_name = property_exists($node, 'experiment_name') ? property_exists($node, 'bioproject_name') : '';
    $description     = property_exists($node, 'description') ? property_exists($node, 'description') : '';
    $bioproject_id   = property_exists($node, 'bioproject_id') ? property_exists($node, 'bioproject_id') : '';
    $biosample_id    = property_exists($node, 'biosample_id') ? property_exists($node, 'biosample_id') : '';
    $rank            = property_exists($node, 'rank') ? property_exists($node, 'rank') : '0';
    $experiment_id   = NULL;
  }

  // === construct bioproject form ===
  $form['experiment_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Experiment Name (Biosample_name + rep Num)'),
    '#required' => TRUE,
    '#default_value' =>  $experiment_name,
  );
  $form['description'] = array(
    '#type' => 'textarea',
    '#rows' => 8,
    '#title' => t('Experiment Description'),
    '#default_value' => $description,
  );

  // Obtain the list of contacts from the chado contact table. 
  $sql = 'SELECT P.project_id, P.name FROM chado_bioproject B 
   LEFT JOIN {project} P ON P.project_id = B.project_id ORDER BY P.name';
  $results = chado_query($sql);
  $projects = array();
  while ($project = $results->fetchObject()) {
    $projects[$project->project_id] = $project->name;
  }

  $lt_message = tripal_set_message(t('If the bioproject is not present in the dropdown list, ')
    . l(t('add a new SRA Bioproject.'), 'node/add/chado-bioproject'),
    TRIPAL_INFO, array('return_html' => TRUE)
  );

  $form['bioproject_id'] = array(
    '#title' => t('BioProject'),
    '#type' => 'select',
    '#empty_value' => '', 
    '#description' => t('Please select the BioProejct of this Experiment.'),
    '#default_value' => $bioproject_id,
    '#required' => TRUE,
    '#options' => $projects,
    '#suffix' => $lt_message,
  );

  $sql = 'SELECT M.biomaterial_id, M.name FROM chado_biosample S
    LEFT JOIN {biomaterial} M ON M.biomaterial_id = S.biomaterial_id ORDER BY M.name';
  $results = chado_query($sql);
  $biosamples = array();
  while ($sample = $results->fetchObject()) {
    $biosamples[$sample->biomaterial_id] = $sample->name;
  }

  $lt_message = tripal_set_message(t('If the biosample is not present in the dropdown list, ')
    . l(t('add a new SRA Biosample.'), 'node/add/chado-biosample'),
    TRIPAL_INFO, array('return_html' => TRUE)
  );

  $form['biosample_id'] = array(
    '#title' => t('BioSample'),
    '#type' => 'select',
    '#empty_value' => '',
    '#description' => t('Please select the BioSample of this Experiment.'),
    '#default_value' => $biosample_id,
    '#required' => TRUE,
    '#options' => $biosamples,
    '#suffix' => $lt_message,
  );

  $form['rank'] = array(
    '#title' => t('Rank (Biological Replicate Number)'),
    '#type' => 'textfield',
    '#empty_value' => '',
    '#default_value' => $rank,
    '#required' => TRUE,
  );

  // construct experiment forms using experiment specific cv
  $exp_cv = array(
    'experiment_platform', 
    'experiment_strategy', 
    'experiment_source', 
    'experiment_selection', 
    'experiment_layout'
  );
  $node->experiment_cv = $exp_cv; // pass experiment cv term using node

  foreach ($exp_cv as $cv_name) {
    // check if the cvterm is selected 
    $cv_value = property_exists($node, $cv_name) ? $node->$cv_name : '';

    $cv = tripal_get_cv(array('name'=>$cv_name));
    $cv = chado_expand_var($cv, 'table', 'cvterm');

    $cv_options = array(
      '' => '-Select-',   
    );
    foreach ($cv->cvterm as $cvterm) {
      $cv_options[$cvterm->cvterm_id] = $cvterm->name;
    }

    $form[$cv_name] = array(
      '#title' => tripal_bioproject_cvterm_display($cv_name,1),
      '#type' => 'select',
      '#options' => $cv_options,
      '#default_value' => $cv_value,
      '#description' => $cv->definition,
      '#required' => TRUE,
    );
  }

  // PROPERTIES FORM
  //---------------------------------------------
  $prop_cv = tripal_get_default_cv('experimentprop', 'type_id');
  $cv_id = $prop_cv ? $prop_cv->cv_id : NULL;
  $details = array(
    'property_table' => 'experimentprop',   // the name of the prop table
    'chado_id' => $experiment_id,           // the value of project_id for this record
    'cv_id' => $cv_id                       // the cv.cv_id of the cv governing projectprop.type_id
  );
  chado_add_node_form_properties($form, $form_state, $details);
  return $form;
}

/**
 * Implements hook_validate().
 * 
 * @ingroup tripal_experiment
 */
function chado_experiment_validate($node, $form, &$form_state) {
  // Don't validate if a delete.
  if ($node->op == 'Delete') {
    return;
  }

  // We are syncing if there is an experiment_id but no node ID. In this case a 
  // validation is not required.  
  if (is_null($node->nid) and property_exists($node, 'experiment_id') and $node->experiment_id != 0) {
    return;
  } 

  // Remove white space around field values.
  $node->experiment_name = trim($node->experiment_name);
  $node->description = trim($node->description);

 // Validate for an update.  
  if (property_exists($node, 'experiment_id')) {
    $sql = "
      SELECT * 
      FROM {experiment} 
      WHERE
        name = :name AND NOT
        experiment_id = :experiment_id
    "; 
    $args = array(':name' => $node->experiment_name, ':experiment_id' => $node->experiment_id);
    $result = chado_query($sql, $args)->fetchObject(); 
    if ($result) {
      form_set_error('experiment_name', t("Update cannot proceed. The experiment
        '$node->experiment_name' is already present in the database."));
      tripal_report_error('tripal_sra', TRIPAL_WARNING,
        'Update experiment: name already exists: %value', 
        array('%value' => "name = $node->experiment_name"));
    }
  }
  // Validate for an insert.
  else {
    $value = array(
      'name' => $node->experiment_name,
    );
    $experiment = chado_select_record('experiment', array('experiment_id'), $value);
    if (sizeof($experiment) > 0) {
      form_set_error('experiment_name', 'Cannot add the experiment with this name. 
        The '.$node->bioproject_name.' already exists.');
      tripal_report_error('tripal_sra', TRIPAL_WARNING,
        'Insert experiment: name already exists: %value',
        array('%value' => "name = $node->bioproject_name")); 
    }
  }     
}  

/**
 * Implements hook_insert().
 * 
 * This function is called on insert of a new node
 * of type 'chado_experiment' and inserts the necessary information to 
 * public chado_experiment table. 
 * 
 * @ingroup tripal_experiment
 */
function chado_experiment_insert($node) {
  
  // Remove white space around field values.
  $node->experiment_name = trim($node->experiment_name);
  $node->description = trim($node->description);

  // Check to see if the node is already associated with a chado record. 
  if (!property_exists ($node, 'experiment_id')) {
    $values = array(
      'name' => $node->experiment_name,
      'description' => $node->description,
    );

    $values['project_id'] = $node->bioproject_id;
    $values['biomaterial_id'] = $node->biosample_id;
    $experiment = chado_insert_record('experiment', $values);
    if (!$experiment) {
      drupal_set_message(t('Unable to add experiment.'), 'warning');
      tripal_report_error('tripal_sra', TRIPAL_ERROR, 'Insert Experiment: Unable to create experiment where values: %values',
      array('%values' => print_r($values, TRUE)));
      return;
    } 
    $experiment_id = $experiment['experiment_id'];

    // * Properties Form *
    $details = array(
      'property_table' => 'experimentprop',    // the name of the prop table
      'base_table' => 'experiment',            // the name of your chado base table
      'foreignkey_name' => 'experiment_id',    // the name of the key in your base table
      'foreignkey_value' => $experiment_id     // the value of the experiment_id key
    );
    chado_update_node_form_properties($node, $details);

    // insert Experiment Properties to experimentprop table
    foreach ($node->experiment_cv as $cv_name) {
      $values = array(
        'experiment_id' => $experiment_id,
        'type_id' => $node->$cv_name,
        'value' => null,
        'rank' => 0,
      );
      $cv_insert = chado_insert_record('experimentprop', $values);
      if (!$cv_insert) {
        drupal_set_message(t('Unable to insert Experiment Properties.'), 'warning');
        tripal_report_error('tripal_sra', TRIPAL_ERROR, 'Insert Experiment Properties: Unable to insert Experiment where values: %values',
        array('%values' => print_r($values, TRUE)));
      }
    }
  }
  else {
    $experiment_id = $node->experiment_id;
  } 

  // Check to see if the chado entry has a corresponding chado_bioproject entry.
  // Add if no corresponding entry exists in the chado_bioproject table. 
  $check_org_id = chado_get_id_from_nid('experiment', $node->nid, 'chado_experiment');

  if (!$check_org_id) {
    $record = new stdClass();
    $record->nid = $node->nid;
    $record->vid = $node->vid;
    $record->experiment_id = $experiment_id;
    drupal_write_record('chado_experiment', $record);
  }	
} 

/**
 * Implements hook_load().
 * 
 *  When a node is requested by the user this function is called to allo us 
 *  to add auxiliary data to the node object.  
 * 
 * @ingroup tripal_experiment
 */
function chado_experiment_load($nodes) {

  foreach ($nodes as $nid => $node) {
    // Find the experiment nodes and add details. 
    $experiment_id = chado_get_id_from_nid('experiment', $nid, 'chado_experiment');
    // Ignore orphaned nodes.
    if (!$experiment_id) {
      continue;
    }

    // Build the bioproject variable.
    $values = array('experiment_id' => $experiment_id);
    $experiment = chado_generate_var('experiment', $values);
    $experiment = chado_expand_var($experiment, 'table', 'experimentprop'); 

    // Add the description field,
    $nodes[$nid]->experiment = $experiment;

    if (sizeof($experiment->experimentprop) > 0 ) {
      foreach ($experiment->experimentprop as $ep) {
        $cv_name = $ep->type_id->cv_id->name;
        $cv_value = $ep->type_id->cvterm_id;
        $nodes[$nid]->$cv_name = $cv_value;
      }
    }

    // Get title the node title.
    //$node->title = chado_get_node_title($node); 
    $node->title = $experiment->name;
  } 
} 

/**
 * Implements [content_type]_chado_node_default_title_format().
 */ 
function chado_experiment_chado_node_default_title_format() {
  return '[experiment.name]';
} 

/**
 * Implements hook_chado_node_default_url_format(). 
 */
function chado_experiment_chado_node_default_url_format() {
  return '/experiment/[experiment.experiment_id]';
} 

/**
 * Implements hook_node_insert(). 
 * Acts on all content types. 
 * 
 * @ingroup tripal_experiment
 */ 
function tripal_experiment_node_insert($node) {
  dpm($node, 'insert node1');
  switch ($node->type) {
    case 'chado_experiment':
      dpm($node, 'insert node2');
      // Find the a experiment and add details.     
      $experiment_id = chado_get_id_from_nid('experiment', $node->nid, 'chado_experiment');
      $values = array('experiment_id' => $experiment_id);
      $experiment = chado_generate_var('experiment', $values);
      $node->experiment = $experiment;
     
      // Set the node title.  
      // $node->title = chado_get_node_title($node);
      $node->title = $experiment->name;     
      // Set the URL path.  
      chado_set_node_url($node);
    
      break;
  }
}

/**
 * Implements hook_node_update().
 * Acts on all content types.
 * 
 * @ingroup tripal_experiment
 */
function tripal_experiment_node_update($node) {
  dpm($node, 'update node1');
  switch ($node->type) {
    case 'chado_experiment':
      dpm($node, 'update node2');      
      // Set the node title.  
      $node->title = chado_get_node_title($node);
      $node->title = $experiment->name; 
      // Set the URL path.  
      chado_set_node_url($node);
    
      break; 
  }
} 

/**
 * Implements hook_update().
 * 
 * @ingroup tripal_experiment
 */
function chado_experiment_update($node) {

  // Remove white space around field values.
  $node->experiment_name = trim($node->experiment_name);
  $node->description = trim($node->description);

  $experiment_id = chado_get_id_from_nid('experiment', $node->nid, 'chado_experiment');
 
  if ($node->revision) {
    // Chado does not do revisions but following the example of prior modules
    // keep this here for when/if revisions are addressed in the future. 
  }
  $match = array(
    'experiment_id' => $experiment_id,
  );

  $bioproject_id = $node->bioproject_id;
  $biosample_id = $node->biosample_id;
  $rank = $node->rank;

  $values = array(
    'name' => $node->experiment_name,
    'description' => $node->description,
    'project_id' => $bioproject_id,
    'biomaterial_id' => $biosample_id,
    'rank' => $rank
  );

  $status = chado_update_record('experiment', $match, $values);

  if (!$status) {
    drupal_set_message(t('Unable to update experiment.'), 'warning');
    tripal_report_error('tripal_sra', TRIPAL_WARNING, 'Update experiment: Unable to update experiment where values: %values', array('%values' => print_r($values, TRUE)));
  } 
    
  // * Properties Form *
  $details = array(
    'property_table' => 'experimentprop',    // the name of the prop table
    'base_table' => 'experiment',            // the name of your chado base table
    'foreignkey_name' => 'experiment_id',    // the name of the key in your base table
    'foreignkey_value' => $experiment_id     // the value of the experiment_id key
  );
  chado_update_node_form_properties($node, $details);

  // insert Experiment Properties to experimentprop table
  foreach ($node->experiment_cv as $cv_name) {
    $values = array(
      'experiment_id' => $experiment_id,
      'type_id' => $node->$cv_name,
      'value' => null,
      'rank' => 0,
    );
    $cv_insert = chado_insert_record('experimentprop', $values);
    if (!$cv_insert) {
      drupal_set_message(t('Unable to insert Experiment Properties.'), 'warning');
      tripal_report_error('tripal_sra', TRIPAL_ERROR, 'Insert Experiment Properties: Unable to insert Experiment where values: %values',
      array('%values' => print_r($values, TRUE)));
    }
  }
} 

/**
 * Implements hook_delete().
 * 
 * Delete experiment from both drupal and chado databases. Check dependencies before
 * deleting from chado. 
 * 
 * @ingroup tripal_experiment
 */
function chado_experiment_delete($node) {
  $experiment_id = chado_get_id_from_nid('experiment', $node->nid, 'chado_experiment');
  // If there is no bioproject for this node, this is not the correct type or the chado
  // entry is lost. 
  if (!$experiment_id) {
    return;
  }

  // Remove data from drupal tables including chado_bioproject, node, and node_revisions
  $sql_del = "DELETE FROM {chado_experiment} WHERE nid = :nid AND vid = :vid";
  db_query($sql_del, array(':nid' => $node->nid, ':vid' => $node->vid));
  $sql_del = "DELETE FROM {node} WHERE nid = :nid AND vid = :vid";
  db_query($sql_del, array(':nid' => $node->nid, ':vid' => $node->vid));
  $sql_del = "DELETE FROM {node_revision} WHERE nid = :nid AND vid = :vid";
  db_query($sql_del, array(':nid' => $node->nid, ':vid' => $node->vid));

  chado_delete_record('experimentprop', array('experiment_id' => $experiment_id));
  chado_delete_record('experiment', array('experiment_id' => $experiment_id));
  drupal_set_message(t("Warning: the experiment ".$experiment_id." has been removed from chado database."), 'status');
} 
