<?php
/**
 * @file 
 * Administration interface for deleting multiple experiments.
 */ 

/**
 * A form for indicating the experiments to delete.
 * 
 * @ingroup tripal_experiments
 */
function tripal_experiment_delete_form() {

  // Get the list of project from chado.project.
  $sql = "SELECT CP.project_id, CP.name FROM chado_project P 
    LEFT JOIN {project} CP ON CP.project_id = P.project_id
    ORDER BY CP.name";
  $bioproject_rset = chado_query($sql);
  $bioprojects = array();
  while ($bioproject = $bioproject_rset->fetchObject()) {
    $bioprojects[$bioproject->project_id] = "$bioproject->name";
  }

  $form['bioproject_id'] = array(
    '#title' => t('BioProject'),
    '#type' => 'select',
    '#options' => $bioprojects,
    '#prefix' => '<div style="height: 400px; overflow: scroll">',
    '#suffix' => '</div><br>',
  );

  // Get the list of experiment from chado.experiment.
  $sql = "SELECT * FROM {experiment} ORDER BY name";
  $biomaterial_rset = chado_query($sql);
  $biomaterials = array();
  while ($biomaterial = $biomaterial_rset->fetchObject()) {
    $biomaterials[$biomaterial->biomaterial_id] = "$biomaterial->name";
  }

  // Create the delete form.
  $form['desc'] = array(
    '#markup' => t("Use the following form to delete biosamples which may or may
        not be synced."),
  );

  $form['biosample_names'] = array(
    '#type' => 'textarea', 
    '#title' => t('Biosample Names'),
    '#description' => t('Please provide a list of biosample names
       (separated by spaces or new lines) to be deleted. If you specify biosample
       names then all other options below will be ignored.'),
   );

  $organisms = tripal_get_organism_select_options(FALSE); 
  $form['organism_id'] = array(
    '#title' => t('Organism'),
    '#type' => 'select',
    '#description' => t("Choose the organism for which biosamples will be deleted."),
    '#options' => $organisms,
  );

  // kentnf: change this later, for delecte biosamples for projects
  $analyses = tripal_get_analysis_select_options(FALSE);
  $form['analysis_id'] = array(
    '#title' => t('Analysis'),
    '#type' => 'select',
    '#description' => t("Choose the organism for which biosamples will be deleted"),
    '#options' => $analyses,
  );

  $form['button'] = array(
    '#type' => 'submit',
    '#value' => t('Delete Experiments'),
  );
  return $form;
}

/**
 * Validation for the delete biosamples form.
 * 
 * @ingroup tripal_experiment
 */
function tripal_experiment_delete_from_validate($form, &$form_state) { 
  $biosample_names = $form_state['values']['biosample_names'];
  $bioproject_id = $form_state['values']['bioproject_id'];

  if (!$biosample_names and !$bioproject_id) {
    form_set_error('biosample_names', t("Please select at least one option"));
  }
} 

/**
 * Submit the delete experiments form.
 * 
 * @ingroup tripal_experiment 
 */
function tripal_experiment_delete_form_submit($form, &$form_state) {
  global $user;

  $biosample_names = $form_state['values']['biosample_names'];
  $bioproject_id = $form_state['values']['organism_id'];

  $args = array($biosample_names, $bioproject_id);
  tripal_add_job("Delete Experiments", 'tripal_sra',
    'tripal_experiment_delete_experiments', $args, $user->uid);
}

/**
 * Function that is called by tripal jobs that actually delete the 
 *  indicated bioexperiments.
 * 
 * @param $biosample_names
 *   (Optional) The list of biosample names to delete.
 * @param $organism_id
 *   (Optional) The organism_id of the biosamples to delete. 
 * @param $analysis_id
 *   (Optional) The analysis_id of the biosamples to delete. 
 * @param $job
 *   The tripal_job id
 * 
 * @ingroup tripal_biomaterial
 */
function tripal_experiment_delete_experiment($biomaterial_names, $organism_id, $analysis_id, $job = NULL) {

  global $user;

  // Begin the database transaction.
  $transaction = db_transaction();
  print "\nNOTE: Deleting biomaterials is performed using a database transaction. \n" . 
      "If the delete fails or is terminated prematurely then the entire set of \n" . 
      "deletions is rolled back the biomaterials will continue to be found in the database\n\n";

  try {
    // If biomaterial names have been provided then handle those.
    if ($biomaterial_names) {
      $names = preg_split('/\s+/', $biomaterial_names);
      if (sizeof($names) == 1) { 
        $names = $names[0];
      }
      $match['name'] = $names;
      $num_deletes = chado_select_record('biomaterial', array('count(*) as cnt'), $match);
      print "Deleting " . $num_deletes[0]->cnt . " features\n";

      chado_delete_record('biomaterial', $match);
      return;
    }

    // Now handle the combination of all other inputs. 
    $args = array();
    $sql = "";
    $join = "";
    $where = "";
    if ($organism_id) { 
      $where .= 'AND B.taxon_id = :organism_id ';
      $args[':organism_id'] = $organism_id;
    }
    if ($analysis_id) {
      $join .= 'INNER JOIN {assay_biomaterial} AB ON B.biomaterial_id = AB.biomaterial_id ';
      $join .= 'INNER JOIN {assay} AY ON AB.assay_id = AY.assay_id ';
      $join .= 'INNER JOIN {acquisition} AC ON AY.assay_id = AC.assay_id ';
      $join .= 'INNER JOIN {quantification} Q ON AC.acquisition_id = Q.acquisition_id ';
      $where .= 'AND AC.analysis_id = :analysis_id';
      $args[':analysis_id'] = $analysis_id;
    }

    // Only perform a delete if the where clause is not empty. Other wise "1=1"
    // in the sql statement below will cause all biomaterials to be deleted.
    if (!$where) { 
      throw new Exception('Cannot delete biomaterials as no filters are available');
    }
    // First, count the number of records to be deleted. 
    $sql = "
      SELECT count(B.biomaterial_id)
      FROM {biomaterial} B
        $join
      WHERE 1=1 $where";
    $num_deleted = chado_query($sql, $args)->fetchField();
    // Second, delete the records.
    $sql = "
      DELETE FROM {biomaterial} WHERE biomaterial_id IN ( 
        SELECT B.biomaterial_id
        FROM {biomaterial} B
          $join
        WHERE 1=1 $where
      )";
    chado_query($sql,$args);
    print "Deletion completed successfully. Deleted $num_deleted biomaterial(s).\n";

    print "Now removing orphaned biomaterial pages\n";
    chado_cleanup_orphaned_nodes('biomaterial', $num_deletes[0]->cnt, 'chado_biomaterial', 'chado_biomaterial');
  }
  catch (Exception $e) {
    print "\n"; 
    $transaction->rollback();
    print "FAILED: Rolling back database changes...\n";
    watchdog_exception('tripal_biomaterial', $e); //*** does this need to be tripal_analsis_exp..
    return 0;
  }
  print "\nDONE\n";
}

