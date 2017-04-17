<?php
$node = $variables['node'];
$bioproject = $variables['node']->bioproject;
$bioproject = chado_expand_var($bioproject,'field','project.description'); ?>

<div class="tripal_bioproject-teaser tripal-teaser">
  <div class="tripal-bioproject-teaser-title tripal-teaser-title"><?php
    print l($node->title, "node/$node->nid", array('html' => TRUE));?>
  </div>
  <div class="tripal_bioproject-teaser-text tripal-teaser-text"><?php
    print substr($bioproject->description, 0, 650);
    if (strlen($bioproject->description > 650)) {
      print "... " . l("[more]", "node/$node->nid");
    } ?>
  </div>
</div>

