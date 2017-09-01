<?php
//dpm($variables); // debug

// This conditional is added to prevent errors in the biosample TOC admin page.
if (property_exists($variables['node'],'biosample')) {
  $biosample = $variables['node']->biosample;
  $biosample = chado_expand_var($biosample, 'field', 'biomaterial.description');

  ?>
  <div class="tripal_biomaterial-data-block-desc tripal-data-block-desc"></div>
  <?php

  $headers = array();
  $rows = array();
 
  // The biosample name.
  $rows[] = array(
    array(
      'data' => 'Biosample',
      'header' => TRUE,
      'width' => '20%',
    ),
    $biosample->name
  );

  // The organism from which the biosample was collected
  if($biosample->taxon_id) {
    $organism =  '<i>' . $biosample->taxon_id->genus . ' ' . $biosample->taxon_id->species . '</i>';
    if (property_exists($biosample->taxon_id, 'nid')) {
      $organism =  l('<i>' . $biosample->taxon_id->genus . ' ' . $biosample->taxon_id->species . '</i>', 'node/' . $biosample->taxon_id->nid, array('html' => TRUE));
    }
    $rows[] = array(
      array(
        'data' => 'Organism',
        'header' => TRUE,
        'width' => '20%',
      ),
    $organism
    );
  } 

  // The biosource provider
  if($biosample->biosourceprovider_id) {
    $rows[] = array(
      array(
        'data' => 'Biosample Provider',
        'header' => TRUE,
        'width' => '20%',
      ),
      '<i>' . $biosample->biosourceprovider_id->name . '</i>'
    );
  }

  // allow site admins to see the biomaterial ID
  if (user_access('view ids')) {
    // Biomaterial ID
    $rows[] = array(
      array(
        'data' => 'BioSample ID',
        'header' => TRUE,
        'class' => 'tripal-site-admin-only-table-row',
      ),
      array(
        'data' => $biosample->biomaterial_id,
        'class' => 'tripal-site-admin-only-table-row',
      ),
    );
  }

  // Generate the table of data provided above. 
  $table = array(
    'header' => $headers,
    'rows' => $rows,
    'attributes' => array(
      'id' => 'tripal_biomaterial-table-base',
      'class' => 'tripal-biomaterial-data-table tripal-data-table',
    ),
    'sticky' => FALSE,
    'caption' => '',
    'colgroups' => array(),
    'empty' => '',
  );

  // Print the table and the description.
  print theme_table($table); 

  // Print the biomaterial description.
  if ($biosample->description) { ?>
    <div style="text-align: justify"><?php print $biosample->description?></div> <?php
  }



  /**
   * display properties
   */
  $biosample = chado_expand_var($biosample, 'table', 'biomaterialprop', array('return_array' => 1));
  $properties = $biosample->biomaterialprop;

  if (count($properties) > 0) { 
    $headers = array('Property Name', 'Value');
    $rows = array();
    foreach ($properties as $property) {
      $rows[] = array(
        ucfirst(preg_replace('/_/', ' ', $property->type_id->name)),
        $property->value
      );
    }

    $table = array(
      'header' => $headers,
      'rows' => $rows,
      'attributes' => array(
        'id' => 'tripal_biosample-table-properties',
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

