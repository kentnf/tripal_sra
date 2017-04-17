<?php

dpm($variables, 'bioproject_experiments'); // debug

// This line is added so that no errors show in the TOC admin page. 
if (property_exists($variables, 'experiment')) { 
  $experiments = $variables->experiment;

  if (count($experiments) > 0) {
    ?>
    <div class="tripal_bioproject-data-block-desc tripal-data-block-desc">Additional information about this biomaterial:</div>
    <?php
  
    $headers = array('Experiment Name', 'Description', 'BioSample', 'Organism');
    $rows = array();

    foreach ($experiments as $experiment) {
      $rows[] = $experiment;
    }
  
    $table = array(
      'header' => $headers,
      'rows' => $rows,
      'attributes' => array(
        'id' => 'tripal_biomaterial-table-properties',
        'class' => 'tripal-data-table table'
      ),
      'sticky' => FALSE,
      'caption' => '',
      'colgroups' => array(),
      'empty' => '',
    );
  
    print theme_table($table);
  }
} 
?>
