<?php
/**
 * @file
 */

/**
 * Implements an admin page to congigure the display of analysis: expression data.
 * 
 * @ingroup tripal_sra
 */
//error_reporting(E_ALL);
function tripal_experiment_loader($form, &$form_state) {

  // Set the breadcrumbs. 
  $breadcrumb = array(); //*** update this for tripal
  $breadcrumb[] = l('Home', '<front>');
  $breadcrumb[] = l('Administration', 'admin');
  $breadcrumb[] = l('Tripal', 'admin/tripal');
  $breadcrumb[] = l('Extensions', 'admin/tripal/extension');
  $breadcrumb[] = l('Tripal SRA', 'admin/tripal/extension/tripal_sra');
  $breadcrumb[] = l('Experiment Loader', 'admin/tripal/extension/tripal_sra/experiment_loader');
  drupal_set_breadcrumb($breadcrumb);

  $organism_id  = '';

  $form['#tree'] = TRUE;

  $form['description'] = array( //***
    '#markup' => t('This loader is designed to load data from NCBI BioSample compatible formats.
      If you have loaded data into NCBI, download download an xml file from NCBI with your 
      BioSamples. This will ensure biomaterials have the BioSample accession number. 
      If you have not uploaded to NCBI, load a flatfile in the NCBI BioSample Submission format. 
      The NCBI BioSample submission format can be downloaded 
      <a href="https://submit.ncbi.nlm.nih.gov/biosample/template/" target = "_blank">here</a>. 
      Available attributes to use can be found 
      <a href="http://www.ncbi.nlm.nih.gov/biosample/docs/attributes/" target = "_blank">here</a>. 
      You may add additional fields to the file, by adding another column to the submission file. 
      The header of the column will be used as the property type. <b>Biomaterials must have a unique
      sample_name.</b><br><br>To associate ontology terms (e.g., Plant Ontology, Human Disease Ontology) ') 
      . t('<a href="@url">load an OBO file</a>', array('@url' => '/admin/tripal/loaders/obo_loader'))
      . t(' and then load terms with the ') 
      . t('<a href="@url">bulk loader</a> ', array('@url' => '/node/add/tripal-bulk-loader'))
      . t('(select "Biomaterial Ontology Term Loader" as the template).') 
      . t(' <b>Caution: If a biomaterial with the same name is already in the database,')
      . t(' this loader will update the biomaterial with the parameters set by this form.</b>'));

  $form['path'] = array(
    '#type' => 'textfield',
    '#title' => t('File Path'),
    '#description' => t('The path to the flat file. /var/www/html/icugi3rd/sites/all/modules/tripal_rnaseq/experiment_Fake_PI.txt'),
    '#required' => TRUE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit Job'),
  );

  return $form;
}

/**
 * Implements hook_validate
 * 
 * @ingroup tripal_sra
 */
function tripal_experiment_loader_validate($form, &$form_state) {
  $path = $form_state['values']['path'];
  if ($path == '') { form_set_error('path', 'Please enter the path to a file.');} //** abs or relative?
  else if (!is_readable($path)) { form_set_error('path', 'File is not present or is unreadable.'); }
}

/**
 * Implement hook_submit 
 * add a tripal job for loading experiment from txt file
 */
function tripal_experiment_loader_submit($form, &$form_state) {

  $path = $form_state['values']['path'];
  global $user;

  $job_args = array(
    $path,
  );

  tripal_add_job("Loading Experiment from txt file: $path", 'tripal_sra',
     'flat_experiment_parser', $job_args, $user->uid); 

  drupal_set_message('done', 'status');
}

/**
 * load experiment from txt file into chado database
 */
function flat_experiment_parser($path) {

  // Check file path to see if file is accessible.nsaction = db_transaction();
  $transaction = db_transaction();
  print "\nNOTE: Loading of this experiment file is performed using a database transaction. \n" .
        "If the load fails or is terminated prematurely then the entire set of \n" .
        "insertions/updates is rolled back and will not be found in the database\n\n";
  try {  
  if (!is_readable($path)) {
    print "ERROR: File is not readable or not present.";
    exit;
  }

  $num_experiments = 0;
  $cur_experiment  = 0;

  $separator = "\t";
  $enclosure = "\0";
  
  // check if the file given is empty
  $fileSize = filesize($path);
  if($fileSize == 0){
     print("ERROR: File at $path is empty. Try again with a new file.\n");
     exit;
  }

  // check the headers of Experiment.
  // 1. get headers -- 1st line with sample_name
  // 2. Make sure there is no duplidate headers.
  // 3. parse headers 
  $fp = fopen($path, "r");
  while($line = fgetcsv($fp,0,$separator,$enclosure)) {
    foreach($line as $field) {
      if(preg_match("/(bioproject_accession)/",$field)) {
        break 2;
      }
    }
  }
 
  $headers = $line;
  if (count($header_repeats = array_diff_assoc($headers, array_unique($headers))) > 0 ) {
    foreach ($header_repeats as $repeat) {
      if ($repeat != 'filetype' and $repeat != 'filename' and $repeat != 'MD5_checksum') {
        print "ERROR: There are multiple instance of the following "
              . "header(s) in the BioSample file:\n";
        print "  " . $repeat . "\n";
        print "Please make sure there is only one instance of each "
              . "header in the BioSample file\n";
        exit;
      }
    }
  }

  $experiment_headers = [];
  $lib_headers = [];
  $prop_headers = [];
  $skip_props = array(
    'reference_genome_assembly (or accession)', 'alignment_software',
    'filetype', 'filename', 'MD5_checksum');
  $rename_props = array(
    'forward_read_length' => 'planned_read_length_mate_1', 
    'reverse_read_length' => 'planned_read_length_mate_2',
  );

  for ($i=0; $i < count($headers); $i++) {
    $header = trim(str_replace("*","",$headers[$i]));

    // header for bioproject
    if ($header == 'bioproject_accession') {
      $experiment_headers['bioproject_accession'] = $i;
      continue;
    }
    if ($header == 'biosample_accession') {
      $experiment_headers['biosample_accession'] = $i;
      continue;
    }
    if ($header == 'sample_name') {
      $experiment_headers['sample_name'] = $i;
      continue;
    }
    if (preg_match("/(short description)/", $header)) {
      $experiment_headers['short description'] = $i;
      continue;
    }
    if (preg_match("/(design_description)/", $header)) {
      $experiment_headers['design_description'] = $i;
      continue;
    }
    if ($header == 'rank') {
      $experiment_headers['rank'] = $i;
      continue;
    }
    if ($header == 'library_ID') {
      $experiment_headers['library_ID'] = $i;
      continue;
    }

    // header for library
    if (preg_match("/(library_strategy)/",$header)) {
      $lib_headers['experiment_strategy'] = $i;
      continue;
    }
    if (preg_match("/(library_source)/",$header)) {
      $lib_headers['experiment_source'] = $i;
      continue;
    }
    if (preg_match("/(library_selection)/",$header)) {
      $lib_headers['experiment_selection'] = $i;
      continue;
    }
    if (preg_match("/(library_layout)/",$header)) {
      $lib_headers['experiment_layout'] = $i;
      continue;
    }
    if (preg_match("/(platform)/",$header)) {
      //$lib_headers['experiment_platform'] = $i;
      continue;
    }
    if (preg_match("/(instrument_model)/",$header)) {
      $lib_headers['experiment_platform'] = $i;
      continue;
    }

    // current unused prop
    if (in_array($header, $skip_props)) {
      continue;
    }
    else {
      // change the name of column title
      $header =  !empty($rename_props[$header]) ? $rename_props[$header] : $header;
      $values = array(
        'name' => $header,
        'cv_id' => array(
          'name' => 'experiment_property',
        ),
      );
      $result = chado_select_record('cvterm', array('cvterm_id'), $values);
      if (sizeof($result) != 1) {
        //drupal_set_message("$header not in cvterm", 'error');
        print "The Experiment Property " . $header . "is not cvterm\n";
        exit;
      }
      $prop_headers[$header] = $i;
    }
  }

  //dpm($headers, 'header');
  //dpm($experiment_headers, 'experiment');
  //dpm($lib_headers, 'lib');
  //dpm($prop_headers, 'prop');

  // parse experiment
  // 1. Count the number of experiment in the file. print error message and exit if there's no experiment
  // 2. Make sure there is no duplicate (bioproject + biosample + rank). 
  $experiment_uniq = [];
  $lines = array();
  while($line = fgetcsv($fp,0,$separator,$enclosure)) {
    $num_experiments++;
    $bioproject_id = find_exp_bioproject($line[$experiment_headers['bioproject_accession']]);
    $biosample_id  = find_exp_biosample($line[$experiment_headers['biosample_accession']]);
    if ($experiment_headers['rank']) {
      $rank = $line[$experiment_headers['rank']];
    } else {
      $rank = 0;
    }
    $experiment_uniq[] = $bioproject_id .'#'. $biosample_id .'#'. $rank;
    $lines[] = $line;

    // check the cvterm of experiment related to library 
    foreach ($lib_headers as $name => $index) {
      $value_term = str_replace(' ', '_', $line[$index]);
      $values = array(
        'name' => $value_term,
        'cv_id' => array(
          'name' => $name,
        ),
      );
      $result = chado_select_record('cvterm', array('cvterm_id'), $values);
      if (sizeof($result) != 1) {
        // drupal_set_message($value_term . " not in cvterm ". $name , 'error');
        print "cvterm" . $value_term . " is not in cv of ". $name . "\n";
        exit;
      }
    }
  }
  fclose($fp);

  if(sizeof($experiment_uniq) == 0){
    print("ERROR: Wrong file format at $path. File must contain three columns: bioproject_accession, biosample_accession and rank'.\n");
    print("Please try again with a file that contains above mentioned columns, followed by lines of experiment data.\n");
    exit;
  }

  if (count($exp_repeats = array_diff_assoc($experiment_uniq, array_unique($experiment_uniq))) > 0 ) {
    print "ERROR: There are multiple instance of the following "
          . "experiment(s) in the Experiment file:\n";
    foreach ($exp_repeats as $repeat) {
      print "  " . $repeat . "\n";
    }
    print "Please make sure there is only one instance of each "
          . "experiment in the experiment file.\n";
    exit;
  }

  /**
   * parse Experiment lines and load Experiment to database
   */
  $hit = 0;
  foreach ($lines as $line) {
    $cur_experiment++;
    //print "Loading Experiment " . ($cur_experiment) . " of " . $num_experiments . "\n";

    $bioproject_id = '';
    $biosample_id =  '';
    $rank = 0;
    $exp_name = '';
    $description = '';

    //dpm($line);

    // Gather the fields to insert into the biomaterial table. 
    foreach($experiment_headers as $name => $index) {
      if ($name == 'bioproject_accession') { 
        $bioproject_id = find_exp_bioproject($line[$index]);
      }
      if ($name == 'biosample_accession') {
        $biosample_id = find_exp_biosample($line[$index]);
      }
      if ($name == 'rank' and is_numeric($line[$index]) and $line[$index] >= 0) { 
        $rank = $line[$index];
      }
      if ($name == 'sample_name') { 
        $exp_name = $line[$index];
      }
      if ($name == 'short_description') {                       
        $description .= 'Short Description' . $line[$index] . "\n";
      }
      if ($name == 'design_description') {
        $description .= 'Design:' . $line[$index] . "\n";
      }
      if ($name == 'library_ID') {
        $description .= 'Library:' . $line[$index] . "\n";
      }
    }

    // Create or update experiment based on the values given.
    $experiment_id = create_update_experiment($bioproject_id, $biosample_id, $rank, $exp_name, $description);

    foreach ($lib_headers as $name => $index) {
      $value_term = str_replace(' ', '_',$line[$index]);

      $record = array(
        'table' => 'experiment',
        'id' => $experiment_id,
      );
      $property = array(
        'type_name' => $value_term,
        'cv_name' => $name,
        'value' => NULL,
      );
      $options = array(
        'update_if_present' => TRUE,
      );
      chado_insert_property($record, $property, $options);
    }

    // Insert into the biomaterialprop table.
    foreach($prop_headers as $name => $index) { 
      if (!empty($line[$index])) {
        // Insert into database. If the type is already in the database the function will not update. 
        if(!tripal_insert_cvterm(array(
            'name' => $name,
            'definition' => $name,
            'cv_name' => 'experiment_property',
            'is_relationship' => 0,
            'db_name' => 'tripal_sra'),
            array('update_existing' => FALSE))) {
          //drupal_set_message("ERROR");
          print "ERROR: Could not insert biomaterial attribute name into the controlled vocabulary table.";
          exit;
        }

        // change the term experiment_layout
        if (!empty($line[$lib_headers['experiment_layout']]) and 
          $line[$lib_headers['experiment_layout']] == 'FRAGMENT' and 
          $name == 'planned_read_length_mate_1') {
          $name = 'planned_read_length';
        }

        $record = array(
          'table' => 'experiment',
          'id' => $experiment_id,
        );
        $property = array(
          'type_name' => $name,
          'cv_name' => 'experiment_property',
          'value' => $line[$index],
        );
        $options = array(
          'update_if_present' => TRUE,
        );
        chado_insert_property($record, $property, $options);
      }
    }
  }

  }
  catch (Exception $e) {
    print "\n";
    watchdog_exception('T_expression_load', $e);
    $transaction->rollback();
    print "FAILED: Rolling back database changes...\n";
  }
  print "\nDone.\n";
  return;
} 

/**
 * find BioSample
 * @param could be biomaterial_id or name
 * @return biomaterial_id
 */
function find_exp_biosample($biosample) {
  if (is_numeric($biosample)) {
    $biomaterial_id = chado_query('select biomaterial_id from {biomaterial} where biomaterial_id = :biomaterial_id', array(':biomaterial_id' => $biosample))->fetchField();
    if (!$biomaterial_id) {
      print "ERROR: Unable to find BioSample using biomaterial_id: $biosample\n";
      exit;
    }
    return $biosample;
  }
  else {
    $biomaterial_id = chado_query('select biomaterial_id from {biomaterial} where name = :name', array(':name' => $biosample))->fetchField();
    if(!$biomaterial_id) {
      print "ERROR: Unable to find BioSample using name: $biosample\n";  
    }
    return $biomaterial_id;
  }
}

/**
 * find BioProject
 * @param could be project_id or name
 * @return project_id
 */
function find_exp_bioproject($bioproject) {

  if (is_numeric($bioproject)) {
    $project_id = chado_query('select project_id from {project} where project_id = :project_id', array(':project_id' => $bioproject))->fetchField();
    if (!$project_id) {
      print "ERROR: Unable to find BioProject using project_id: $bioproject\n";
      exit;
    }
    return $bioproject;  
  }
  else {
    $project_id = chado_query('select project_id from {project} where name = :name', array(':name' => $bioproject))->fetchField();
    if(!$project_id) {
      print "ERROR: Unable to find BioProject using name: $bioproject\n";
    }
    return $project_id;
  }
}

/**
 * Implements a function to create or update experiment. 
 * @param  
 * @return experiment_id
 * @ingroup tripal_sra
 */
function create_update_experiment($bioproject_id, $biosample_id, $rank, $exp_name, $description) {

  // set default rank value, and prepare insert/update values
  if (empty($rank)) {
    $rank = 0;
  }

  $values = array(
    'biomaterial_id' => $biosample_id,
    'project_id' => $bioproject_id,
    'name' => $exp_name,
    'rank' => $rank,
  );

  if($description) {
    $values['description'] = $description;
  }

  // check if the experiment is exist
  $sql_exp_search = "SELECT * FROM {experiment} 
    WHERE biomaterial_id = :biomaterial_id AND project_id = :project_id AND rank = :rank";
  $arg_exp_search = array(
    ':biomaterial_id' => $biosample_id,
    ':project_id' => $bioproject_id,
    ':rank' => $rank,
  );
  $exp_result = chado_query($sql_exp_search, $arg_exp_search)->fetchObject();

  // if experiment already exists in chado db, print warning message 
  if (!empty($exp_result->experiment_id)){
    print("WARNING: Experiment with name of '$exp_result->name' alreay exists in the database. Overwriting database entries for $exp_result->name...\n");

    $experiment_id = $exp_result->experiment_id;

    print("Updating record for experiment id = $experiment_id\n");
    $experiment = chado_update_record('experiment',
      array('experiment_id' => $experiment_id),
      $values,
      array('return_record' => TRUE));
    if($experiment == FALSE) {
      print "ERROR: Unable to update experiment table\n";
      exit;
    }
  }
  else {
    $experiment = chado_insert_record('experiment',$values);
    if($experiment == FALSE) {
      print "ERROR: Unable to write to experiment table\n";
      exit;
    }
  }

  $experiment_id = $experiment['experiment_id'];
  return $experiment_id;
}

