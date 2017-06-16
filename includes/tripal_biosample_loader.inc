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
function tripal_biosample_loader($form, &$form_state) {

  // Set the breadcrumbs. 
  $breadcrumb = array(); //*** update this for tripal
  $breadcrumb[] = l('Home', '<front>');
  $breadcrumb[] = l('Administration', 'admin');
  $breadcrumb[] = l('Tripal', 'admin/tripal');
  $breadcrumb[] = l('Extensions', 'admin/tripal/extension');
  $breadcrumb[] = l('Tripal SRA', 'admin/tripal/extension/tripal_sra');
  $breadcrumb[] = l('BioSample Loader', 'admin/tripal/extension/tripal_sra/biosample_loader');
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
       
  $organisms = tripal_sra_get_organism_select_options(FALSE);

  $form['organism_id'] = array(
    '#title' => t('Organism'),
    '#type' => 'select',
    '#required' => TRUE,
    '#description' => t('The organism from which the biosamples were collected.'),
    '#options' => $organisms,
    '#default_value' => $organism_id,
  );

  $form['path'] = array(
    '#type' => 'textfield',
    '#title' => t('File Path'),
    '#description' => t('The path to the flat file. /var/www/html/icugi3rd/sites/all/modules/tripal_rnaseq/Plant.1.0.fake_PI.txt'),
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
function tripal_biosample_loader_validate($form, &$form_state) {

  // Make sure an organism is selected.
  $organism_id = $form_state['values']['organism_id'];
  if(!$organism_id) {
    form_set_error('organism_id', 'Please select the organism from which the biomaterials were collected.');
  }

  $path = $form_state['values']['path'];
  if ($path == '') { form_set_error('path', 'Please enter the path to a file.');} //** abs or relative?
  else if (!is_readable($path)) { form_set_error('path', 'File is not present or is unreadable.'); }
}

/**
 * Implement hook_submit 
 * add a tripal job for loading BioSample from txt file
 */
function tripal_biosample_loader_submit($form, &$form_state) {

  $path = $form_state['values']['path'];
  $organism_id = $form_state['values']['organism_id'];

  global $user;

  $job_args = array(
    $path,
    $organism_id,
  );

  // test and debug
  // flat_biosample_parser($path, $organism_id);

  tripal_add_job("Loading BioSamples from txt file: $path", 'tripal_sra',
     'flat_biosample_parser', $job_args, $user->uid); 
}

/**
 * load BioSamples from txt file into chado database
 */
function flat_biosample_parser($path, $org_id) {

  // Check file path to see if file is accessible.nsaction = db_transaction();
  $transaction = db_transaction();
  print "\nNOTE: Loading of this BioSample file is performed using a database transaction. \n" .
        "If the load fails or is terminated prematurely then the entire set of \n" .
        "insertions/updates is rolled back and will not be found in the database\n\n";
  try {  
  if (!is_readable($path)) {
    print "ERROR: File is not readable or not present.";
    exit;
  }

  $num_biosamples = 0;
  $cur_biosample = 0;

  $separator = "\t";
  $enclosure = "\0";
  
  // check if the file given is empty
  $fileSize = filesize($path);
  if($fileSize == 0){
     print("ERROR: File at $path is empty. Try again with a new file.\n");
     exit;
  }

  // check the headers of BioSample.
  // 1. get headers -- 1st line with sample_name
  // 2. Make sure there is no duplidate headers.
  // 3. parse headers 
  $fp = fopen($path, "r");
  while($line = fgetcsv($fp,0,$separator,$enclosure)) {
    foreach($line as $field) {
      if(preg_match("/(sample_name)/",$field)) {
        break 2;
      }
    }
  }
 
  $headers = $line;
  if (count($header_repeats = array_diff_assoc($headers, array_unique($headers))) > 0 ) {
    print "ERROR: There are multiple instance of the following "
          . "header(s) in the BioSample file:\n";
    foreach ($header_repeats as $repeat) {
      print "  " . $repeat . "\n";
    }
    print "Please make sure there is only one instance of each "
          . "header in the BioSample file\n";
    exit;
  }

  $biomaterial_headers = [];
  $prop_headers = [];
  $acc_headers = [];

  for ($i=0; $i < count($headers); $i++) {
    $header = trim(str_replace("*","",$headers[$i]));
    //drupal_set_message("'" . $header . "'");
    switch ($header) {
      case 'sample_name':
        $biomaterial_headers['sample_name'] = $i;
        break;
      case 'organism':
        $biomaterial_headers['organism'] = $i;
        break;
      case 'sample_title':
        $biomaterial_headers['sample_title'] = $i;
        break;
      case 'description':
        $biomaterial_headers['description'] = $i;
        break;
      case 'biomaterial_provider':
        $biomaterial_headers['biomaterial_provider'] = $i;
        break;
      case 'bioproject_accession':
        $acc_headers['bioproject_accession'] = $i;
        break;
      case 'biomaterial_accession':
        $acc_headers['biomaterial_accession'] = $i;
        break;
      case 'sra_accession':
        $acc_headers['sra_accession'] = $i;
        break;
      default:
        $values = array(
          'name' => $header,
          'cv_id' => array(
            'name' => 'biosample_property',
           ),
        );
        $result = chado_select_record('cvterm', array('cvterm_id'), $values);
        if (sizeof($result) != 1) {
          print "The BioSample Property " . $header . "is not cvterm\n";
          exit;
        }
        $prop_headers[$header] = $i;
    }
  }

  // parse biosamples  
  // 1. Count the number of biosample in the file. print error message and exit if there's no biosample, or that there's no "sample_name" column in flat file
  // 2. Make sure there is no duplicate sample_name. 
  $biosample_names = [];
  $lines = array();
  while($line = fgetcsv($fp,0,$separator,$enclosure)) {
    $num_biosamples++;
    $biosample_names[] = $line[$biomaterial_headers['sample_name']];
    $lines[] = $line;
  }
  fclose($fp);

  if(sizeof($biosample_names) == 0){
    print("ERROR: Wrong file format at $path. File must contain a column named 'sample_name'.\n");
    print("Please try again with a file that contains at least one column named 'sample_name' in its header line, followed by lines of biosample data.\n");
    exit;
  }

  if (count($sample_repeats = array_diff_assoc($biosample_names, array_unique($biosample_names))) > 0 ) {
    print "ERROR: There are multiple instance of the following "
          . "sample_name(s) in the BioSample file:\n";
    foreach ($sample_repeats as $repeat) {
      print "  " . $repeat . "\n";
    }
    print "Please make sure there is only one instance of each "
          . "sample_name in the biomaterial file.\n";
    exit;
  }

  /**
   * parse biosample lines and load biosample to database
   */
  $hit = 0;
  foreach ($lines as $line) {
    $cur_biosample++;
    print "Loading BioSample " . ($cur_biosample) . " of " . $num_biosamples . "\n";

    $unique_name = '';
    $description = '';
    $biosourceprovider_id = '';
    $organism_id = '';
    // $dbxref_id = '';

    // Gather the fields to insert into the biomaterial table. 
    foreach($biomaterial_headers as $name => $index) {
      if ($name == 'sample_name') { 
        $unique_name = $line[$index];
      }
      if ($name == 'sample_title') {
        $description .= $line[$index]."\n";
      }
      if ($name == 'description') { 
        $description .= $line[$index];
      }
      if ($name == 'biomaterial_provider' and $line[$index] != "") { 
        $biosourceprovider_id = create_update_biosample_contact($line[$index]);
      }
      if ($name == 'organism') {
        $organism_id = find_biosample_taxon($line[$index]);
        if ($organism_id != $org_id) {
          print "The select organism $org_id do not match the organism of $unique_name: $organism_id\n";
          exit;
        }
      }
    }

    // Create the biomaterial based on the values given.
    $biomaterial_id = create_update_biosample($unique_name, $organism_id, $biosourceprovider_id, $description);

    // Insert into the biomaterialprop table.
    foreach($prop_headers as $name => $index) { 
      if (!empty($line[$index])) {
        // Insert into database. If the type is already in the database the function will not update. 
        if(!tripal_insert_cvterm(array(
          'name' => $name,
          'definition' => '',
          'cv_name' => 'biosample_property',
          'is_relationship' => 0,
          'db_name' => 'tripal_sra'),
          array('update_existing' => FALSE))) {
          //drupal_set_message("ERROR");
          print "ERROR: Could not insert biomaterial attribute name into the controlled vocabulary table.";
          exit;
        }
        ////
        $record = array(
          'table' => 'biomaterial',
          'id' => $biomaterial_id,
        );
        $property = array(
          'type_name' => $name,
          'cv_name' => 'biosample_property',
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
 * find biosample_taxon
 * @param could be organism_id or common_name
 * @return organism_id
 */
function find_biosample_taxon($organism) {

  if (is_numeric($organism)) {
    $organism_id = chado_query('select organism_id from {organism} where organism_id = :organism_id', array(':organism_id' => $organism))->fetchField();
    if (!$organism_id) {
      print "ERROR: Unable to find organism using organism_id: $organism\n";
      exit;
    }
    return $organism;
  }
  else {
    $organism_id = chado_query('select organims_id from {organims} where common_name = :common_name', array(':common_name' => $organism))->fetchField();
    if(!$organism_id) {
      print "Unable to find organism using common_name: $organism\n";  
    }
    return $organism_id;
  }
}

/**
 * create or update biosample contact
 * @param could be exist or new contact_name 
 *  or exist contact_id in chado database
 * @return contact_id 
 */ 
function create_update_biosample_contact($contact_name) {

  if (is_numeric($contact_name)) {
    $contact_id = chado_query('select contact_id from {contact} where contact_id = :contact_id',array(':contact_id' => $contact_name))->fetchField();
    if (!$contact_id) {
      print "ERROR: Unable to find contact using contact_id $contact_name\n";
      exit;
    }
    return $contact_name;
  }
  else {
    $contact_id = chado_query('select contact_id from {contact} where name = :name limit 1',array(':name' => $contact_name))->fetchField();
    if(!$contact_id) {
      $values = array(
        'name' => $contact_name,
      );
      $contact = chado_insert_record('contact', $values);
      if($contact==FALSE) {
        print "ERROR: Unable to write to contact table\n";
        exit;
      }
     $contact_id = $contact['contact_id'];
    }
    return $contact_id;
  }
}

/**
 *
 * Implements a function to create a biomaterial. The name field in the biomaterial, 
 *  is the only field that is required to create a biomaterial record. However, becuase this
 *  is designed to be the primary method by biomaterials are loaded into the table, the function 
 *  provides extra parameters with which to describe the biomaterial. 
 *
 *  @param $biomaterial
 *  The name of the biomaterial for which a record will be created. 
 * 
 *  @param $organism_id
 *  The organism_id of the organism associated with the experiment. 
 * 
 *  @param $biosourceprovider_id
 *  The contact_id of the contact who provided the biomaterial.
 * 
 * @ingroup tripal_sra
 */

function create_update_biosample($biomaterial, $organism_id, $biosourceprovider_id, $biomaterial_description) {

  $biomaterial_result = chado_query('select biomaterial_id, taxon_id, description from {biomaterial} where name = :name limit 1',array(':name' => $biomaterial))->fetchObject();
  $biomaterial_id = null;

  // if sample_name already exists in chado db, print warning message 
  if (!empty($biomaterial_result->taxon_id)){
    print("WARNING: Biomaterial with sample_name of '$biomaterial' alreay exists in the database. Overwriting database entries for $biomaterial...\n");
    $biomaterial_id = $biomaterial_result->biomaterial_id;

    if ($biomaterial_result->taxon_id != $organism_id) {
      $sql = "SELECT common_name, genus, species FROM {organism} WHERE organism_id = :organism_id";
      $analysis_org = chado_query($sql, array(":organism_id" => $organism_id))->fetchObject();
      $biomaterial_org = chado_query($sql, array(":organism_id" => $biomaterial_result->taxon_id))->fetchObject();
      print "ERROR: Mismatch between the organism selected in the loader "
          . "and the organism of a biomaterial in the Chado database.\n"
          . "  Organism selected by loader: " . $analysis_org->genus
          . " " . $analysis_org->species . " (" . $analysis_org->common_name . ")\n"
          . "  Biomaterial organism: " . $biomaterial_org->genus . " " . $biomaterial_org->species . " ("
          . $biomaterial_org->common_name . ")\n"
          . "  On biomaterial: " . $biomaterial . "\n"
          . "Please ensure that the organism selected by the loader "
          . "and the organism for each biomaterial in the file to be loaded "
          . "are the same. If these organisms should not be the same, "
          . "delete the offending biomaterial in Chado or change the name of "
          . "the biomaterial to another unique name.\n";
      exit;
    }
  }

  $values = array(
    'name' => $biomaterial,
    'taxon_id' => $organism_id,
  );

  //if($dbxref_id) {
  //  $values['dbxref_id'] = $dbxref_id;
  //}

  if($biomaterial_description) {
    $values['description'] = $biomaterial_description;
  }

  if($biosourceprovider_id) {
    $values['biosourceprovider_id'] = $biosourceprovider_id;
  }

  if(!$biomaterial_id) {
    $biomaterial = chado_insert_record('biomaterial',$values);
    if($biomaterial==FALSE) {
      print "ERROR: Unable to write to biomaterial table\n";
      exit;
    }
  }

  if($biomaterial_id) {
    print("Updating record for biomaterial id = $biomaterial_id\n");
    $biomaterial = chado_update_record('biomaterial',
      array('name' => $biomaterial),
      $values,
      array('return_record' => TRUE));
    if($biomaterial==FALSE) {
      print "ERROR: Unable to update biomaterial_table\n";
      exit;
    }
  }
  $biomaterial_id = $biomaterial['biomaterial_id'];

  return $biomaterial_id;
}

