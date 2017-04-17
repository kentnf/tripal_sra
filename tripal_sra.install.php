<?php
/**
 * @file
 * Functions used to install the analysis: expression module.
 */

/**
 * Implements install_hook().
 */
function tripal_sra_install() {

  // install schema to chado database

  // Get localization function for installation.
  $t = get_t();

  tripal_sra_add_custom_tables();
  tripal_sra_add_cvterms();

  // Set default cvs.
  tripal_set_default_cv('biomaterialprop', 'type_id', 'biosample_property');
  tripal_set_default_cv('biomaterial_relationship', 'type_id', 'biosample_relationship');
  tripal_set_default_cv('projectprop', 'type_id', 'bioproject_property');
  tripal_set_default_cv('project_relationship', 'type_id', 'bioproject_relationship');
  tripal_set_default_cv('experimentprop', 'type_id', 'experiment_property');

  // These variables are used to set how expression information is displayed.
  /**
  Sets a persistent variable.

  variable_set('chado_feature_expression_hide_expression', 0);
  variable_set('chado_feature_expression_hide_biomaterial_labels', 0);
  variable_set('chado_feature_expression_limit_label_length', 0);
  variable_set('chado_feature_expression_min_num_biomaterials', 0);
  variable_set('chado_feature_expression_expression_display', 'column');
  variable_set('chado_feature_expression_biomaterial_display_width', 15);
  */

  variable_set('comment_chado_bioproject', '0');
  variable_set('comment_chado_biosample', '0');
  variable_set('comment_chado_experiment', '0');

  // Make sure the biomaterial (biosample), and project (bioproject), and 
  // experiment (create in chado schema) chado tables are a set
  // as base tables. This allows the tables to be used as base tables in
  // Drupal Views.
  $update = db_update('tripal_views')
    ->fields(array(
      'base_table' => 1,
    ))
    ->condition('table_name', 'project', '=')
    ->execute();
  $update = db_update('tripal_views')
    ->fields(array(
      'base_table' => 1,
    ))
    ->condition('table_name', 'biomaterial', '=')
    ->execute();
  $update = db_update('tripal_views')
    ->fields(array(
      'base_table' => 1,
    ))
    ->condition('table_name', 'experiment', '=')
    ->execute();
}

/**
 * Create experiment table in chado
 */ 
function tripal_sra_add_custom_tables () {
   
  $schema = array(
    'table' => 'experiment',
    'fields' => array(
      'experiment_id'  => array('type' => 'serial', 'size' => 'big', 'not null' => TRUE),
      'biomaterial_id' => array('type' => 'int', 'size' => 'big', 'not null' => TRUE),
      'project_id'     => array('type' => 'int', 'size' => 'big', 'not null' => TRUE),
      'name'           => array('type' => 'varchar', 'length' => 1023, 'not null' => TRUE),
      'description'    => array('type' => 'text', 'not null' => TRUE),
      'rank'           => array('type' => 'int', 'default' => 1, 'not null' => TRUE),
    ),
    'primary key' => array('experiment_id'),
    'indexes' => array(
       'exp_name_idx' => array('name'),
    ),
    'unique keys' => array(
      'exp_uq' => array('biomaterial_id', 'project_id', 'rank'),
    ),
    'foreign keys' => array(
      'biomaterial' => array(
        'table' => 'biomaterial',
        'columns' => array(
          'biomaterial_id' => 'biomaterial_id',
        ),
      ),
      'project' => array(
        'table' => 'project',
        'columns' => array(
          'project_id' => 'project_id',
        ),
      ),
    )
  );
  chado_create_custom_table('experiment', $schema);

  $schema = array(
    'table' => 'experimentprop',
    'fields' => array(
      'experimentprop_id' => array('type' => 'serial', 'size' => 'big', 'not null' => TRUE),
      'experiment_id'     => array('type' => 'int', 'size' => 'big', 'not null' => TRUE),
      'type_id'           => array('type' => 'int', 'size' => 'big', 'not null' => TRUE),
      'value'             => array('type' => 'text', 'not null' => FALSE),
      'rank'              => array('type' => 'int',  'default' => 0, 'not null' => TRUE),
    ),
    'primary key' => array('experimentprop_id'),
    'unique keys' => array(
      'expprop_uq' => array('experiment_id', 'type_id', 'rank'),
    ),
    'foreign keys' => array(   
      'experiment' => array(
        'table' => 'experiment',
        'columns' => array(
          'experiment_id' => 'experiment_id',
        ),
      ),
      'cvterm' => array(
        'table' => 'cvterm',
        'columns' => array(
          'type_id' => 'cvterm_id',
        ),
      ),
    )
  );

  chado_create_custom_table('experimentprop', $schema);
}

/**
 * Implements hook_schema().
 */
function tripal_sra_schema() {

  $schema['chado_experiment'] = array(
    'fields' => array(
      'vid' => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'default' => 0,
      ),
      'nid' => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'default' => 0,
      ),
      'experiment_id' => array(
        'type' => 'int',
        'not null' => TRUE,
        'default' => 0,
      ),
    ),
    'indexes' => array(
      'experiment_id' => array('experiment_id'),
    ),
    'unique keys' => array(
      'nid_vid' => array('nid', 'vid'),
      'vid' => array('vid'),
    ),
    'primary key' => array('nid'),
  );
  $schema['chado_bioproject'] = array(
    'fields' => array(
      'vid' => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'default' => 0,
      ),
      'nid' => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'default' => 0,
      ),
      'project_id' => array(
        'type' => 'int',
        'not null' => TRUE,
        'default' => 0,
      ),
    ),
    'indexes' => array(
      'project_id' => array('project_id'),
    ),
    'unique keys' => array(
      'nid_vid' => array('nid', 'vid'),
      'vid' => array('vid'),
    ),
    'primary key' => array('nid'),
  );
  $schema['chado_biosample'] = array(
    'fields' => array(
      'vid' => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'default' => 0,
      ),
      'nid' => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'default' => 0,
      ),
      'biomaterial_id' => array(
        'type' => 'int',
        'not null' => TRUE,
        'default' => 0,
      ),
    ),
    'indexes' => array(
      'biomaterial_id' => array('biomaterial_id'),
    ),
    'unique keys' => array(
      'nid_vid' => array('nid', 'vid'),
      'vid' => array('vid'),
    ),
    'primary key' => array('nid'),
  );

  return $schema;
}

/**
 * Add cvterms
 */
function tripal_sra_add_cvterms() {

  // CVs for the bioproject content type.
  tripal_insert_cv(
    'bioproject_property',
    'Contains property terms for bioprojects.'
  );
  tripal_insert_cv(
    'bioproject_relationship',
    'Contains types of relationships between bioprojects.'
  );

  // CVs for the biosample content type.
  tripal_insert_cv(
    'biosample_property',
    'Contains property terms for biosamples.'
  );
  tripal_insert_cv(
    'biosample_relationship',
    'Contains types of relationships between biosamples.'
  );

  tripal_insert_cvterm(array(
    'name' => 'bioproject_accession',
    'def' => 'The accession number of the BioProject(s) to which the BioSample belongs. If the BioSample belongs to more than one BioProject, enter multiple bioproject_accession columns. A valid BioProject accession has prefix PRJN, PRJE or PRJD, e.g., PRJNA12345.',
    'cv_name' => 'biosample_property',
    'db_name' => 'tripal_sra',
  ));

  // insert tripal sra db
  $values = array (
    'name' => 'tripal_sra',
    'description' => 'db for tripal sra module, used for adding cvterm for tripal sra module',
  ); 
  tripal_insert_db($values);

  // reference https://www.ncbi.nlm.nih.gov/books/NBK54015/#BioProject-Help.Submitting_to_BioProject
  // bioproject_data_type
  tripal_insert_cv(
    'bioproject_data_type',
    'A general label indicating the primary study goal.'
  );
  $datatypes = array(
    'Assembly' => 'genome assembly project utilizing already existing sequence data including data that was submitted by a different group',
    'Clone_ends' => 'clone-end sequencing project',
    'Epigenomics' => 'DNA methylation, histone modification, chromatin accessibility datasets',
    'Exome' => 'exome resequencing project',
    'Genome_sequencing' => 'whole, or partial, genome sequencing project (with or without a genome assembly)',
    'Map' => 'project that results in non-sequence map data such as genetic map, radiation hybrid map, cytogenetic map, optical map, and etc.',
    'Metagenome' => 'sequence analysis of environmental samples',
    'Metagenome_assembly' => 'a genome assembly generated from sequenced environmental samples',
    'Other' => 'a free text description is provided to indicate Other data type',
    'Phenotype_or_Genotype' => 'project correlating phenotype and genotype',
    'Proteome' => 'large scale proteomics experiment including mass spec. analysis', 
    'Random_Survey' => 'sequence generated from a random sampling of the collected sample; not intended to be comprehensive sampling of the material.',
    'Targeted_locus' => 'project to sequence specific loci, such as a 16S rRNA sequencing',
    'Transcriptome_or_Gene_expression' => 'large scale RNA sequencing or expression analysis. Includes cDNA, EST, RNA_seq, and microarray.',
    'Variation' => 'project with a primary goal of identifying large or small sequence variation across populations.'
  );

  foreach ($datatypes as $datatype => $description) {
    tripal_insert_cvterm(array(
      'name' => 'dp_' . $datatype,
      'def' => $description,
      'cv_name' => 'bioproject_data_type',
      'db_name' => 'tripal_sra',
    ));
  }
  tripal_insert_cvterm(array(
    'name' => 'bioproject_data_type',
    'def' => 'bioproject_sample_scope other desc',
    'cv_name' => 'bioproject_property',
    'db_name' => 'tripal_sra',
  ));

  //bioproject_sample_scope
  tripal_insert_cv(
    'bioproject_sample_scope',
    'Indicates the scope and purity of the biological sample used for the study.'
  );

  $sample_scopes = array('Monoisolate', 'Multiisolate', 'Multi-species', 'Environment', 'Synthetic', 'Other');
  foreach ($sample_scopes as $scope) {
    tripal_insert_cvterm(array(
      'name' => 'ss_' . $scope,
      'def' => $scope,
      'cv_name' => 'bioproject_sample_scope',
      'db_name' => 'tripal_sra',
    ));
  }
  tripal_insert_cvterm(array(
    'name' => 'bioproject_sample_scope',
    'def' => 'bioproject_sample_scope other desc',
    'cv_name' => 'bioproject_property',
    'db_name' => 'tripal_sra',
  ));

  // bioproject_material
  tripal_insert_cv(
    'bioproject_material',
    'Indicates the type of material that is isolated from the sample for use in the study.'
  );
  $materials = array('Genome', 'Purified_chromosome', 'Transcriptome', 'Phenotype', 'Reagent', 'Proteome', 'Other');
  foreach ($materials as $material) {
    tripal_insert_cvterm(array(
      'name' => 'm_' . $material,
      'def' => $material,
      'cv_name' => 'bioproject_material',
      'db_name' => 'tripal_sra',
    ));
  }
  tripal_insert_cvterm(array(
    'name' => 'bioproject_material',
    'def' => 'bioproject_material other desc',
    'cv_name' => 'bioproject_property',
    'db_name' => 'tripal_sra',
  ));

  // bioproject_capture
  tripal_insert_cv(
    'bioproject_capture',
    'Indicates the scale, or type, of information that the study is designed to generate from the sample material.'
  );
  $captures = array('Whole', 'CloneEnds', 'Exome', 'TargetedLocusLoci', 'RandomSurvey', 'Other');
  foreach ($captures as $capture) {
    tripal_insert_cvterm(array(
      'name' => 'c_' . $capture,
      'def' => $capture,
      'cv_name' => 'bioproject_capture',
      'db_name' => 'tripal_sra',
    ));
  }

  tripal_insert_cvterm(array(
    'name' => 'bioproject_capture',
    'def' => 'bioproject_capture other desc',
    'cv_name' => 'bioproject_property',
    'db_name' => 'tripal_sra',
  ));

  // reference ftp://ftp.sra.ebi.ac.uk/meta/xsd/sra_1_5/SRA.experiment.xsd
  // library strategy -- sequence library technology
  // reference: http://www.ebi.ac.uk/ena/submit/reads-library-strategy
  tripal_insert_cv(
    'experiment_platform',
    'The sequencing platform and instrument model.'
  );
  $platforms = array(
    'Illumina_Genome_Analyzer' => 'Illumina_Genome_Analyzer', 
    'Illumina_Genome_Analyzer_II' => 'Illumina_Genome_Analyzer_II',
    'Illumina_Genome_Analyzer_IIx' => 'Illumina_Genome_Analyzer_IIx',
    'Illumina_HiSeq_2500' => 'Illumina_HiSeq_2500',
    'Illumina_HiSeq_2000' => 'Illumina_HiSeq_2000',
    'Illumina_HiSeq_1000' => 'Illumina_HiSeq_1000',
    'Illumina_MiSeq' => 'Illumina_MiSeq',
    'Illumina_HiScanSQ' => 'Illumina_HiScanSQ',
    'Illumina_NextSeq_500' => 'Illumina_NextSeq_500',
    'Illumina_unspecified' => 'Illumina_unspecified',
    '454_GS' => '454_GS',
    '454_GS_20' => '454_GS_20',
    '454_GS_FLX' => '454_GS_FLX',
    '454_GS_FLX+' => '454_GS_FLX+',
    '454_GS_FLX Titanium' => '454_GS_FLX Titanium',
    '454_GS_Junior' => '454_GS_Junior',
  );

  foreach ($platforms as $platform => $description) {
    tripal_insert_cvterm(array(
      'name' => $platform,
      'def' => $description,
      'cv_name' => 'experiment_platform',
      'db_name' => 'tripal_sra',
    ));
  }

  tripal_insert_cv(
    'experiment_strategy',
    'Sequencing technique intended for this library.'
  );

  $strategies = array(
    'POOLCLONE' => 'Pooled clone sequencing: An assay in which DNA is the input molecule derived from pooled clones (for example BACs and Fosmids) is sequenced using high throughput technology using shotgun methodology.',
    'CLONE' => 'Clone by clone sequencing: An assay in which DNA is the input molecule derived from clones which are mapped then sequenced in small fragments.',
    'CLONEEND' => 'Clone end sequencing: An Assay in which DNA is the input molecule derived from a clone and 5P 3P or both ends of a clone are sequenced.',
    'WGS' => 'Whole genome shotgun sequencing: An assay in which DNA is the input molecule derived from fragmented whole genome preparation is sequenced.',
    'WGA' => 'Whole genome amplification sequencing: Non-PCR amplification of the whole genome, followed by random sequencing.',
    'WCS' => 'Random chromosome sequencing: An assay in which DNA is the input molecule derived from chromosome or other replicon is sequenced.',
    'WXS' => 'Random exon sequencing: An assay in which DNA is the input molecule derived from exons is sequenced.',
    'AMPLICON' => 'Amplicon sequencing: An assay in which a DNA or RNA input molecule amplified by PCR is sequenced.',
    'ChIP-Seq' => 'Direct sequencing of chromatin immunoprecipitates: An assay in which chromatin immunoprecipitation with high throughput sequencing is used to identify the cistrome of DNA-associated proteins.',
    'RNA-Seq' => 'Whole Transcriptome Shotgun Sequencing: Random sequencing of whole transcriptome, also known as Whole Transcriptome Shotgun Sequencing, or WTSS.',
    'MRE-Seq' => 'Methylation-Sensitive Restriction Enzyme Sequencing strategy: An assay in which DNA is the input molecule derived from cleavage of DNA by use of methylation sensitive restriction enzymes to fragment DNA at methylation sites before sequencing.',
    'MeDIP-Seq' => 'Methylated DNA Immunoprecipitation Sequencing strategy: An assay in which DNA is the input molecule derived from an antibody based selection process using antibodies targeting methylated DNA, which is then sequenced using high throughput sequencing technology.',
    'MBD-Seq' => 'Direct sequencing of methylated fractions sequencing strategy: An assay in which DNA is the input molecule derived from a selection process using methyl binding domain protein to enrich for methylated fractions of DNA, then sequenced using high throughput sequencing.',
    'MNase-Seq' => 'Direct sequencing following MNase digestion: An assay in which DNA is the input molecule derived from a micrococcal nuclease digestion followed by high throughput sequencing, A method that distinguishes nucleosome positioning based on the ability of nucleosomes to protect associated DNA from digestion by micrococcal nuclease. Sequenced fragments reveal nucleosome location information about the input DNA.',
    'DNase-Hypersensitivity' => 'Sequencing of DNase-hypersensitive sites to identify regularatory elements: An assay in which DNA is the input molecule derived from a DNase-hypersensitivity digest of chromatin with the aim of identifying regulatory elements.',
    'Bisulfite-Seq' => 'Sequencing following treatment of DNA with bisulfite to determine methylation status: An assay in which DNA is the input molecule derived from a bisulphite treatment of DNA to convert cytosine residues to uracil to determine methylation status.',
    'EST' => 'Single pass sequencing of cDNA templates: An assay in which RNA derived from an Expressed Sequence Tags (ESTs) is sequenced.',
    'FL-cDNA' => 'Full-length sequencing of cDNA templates: An assay in which RNA derived from a full length cDNA template is sequenced.',
    'miRNA-Seq' => 'Micro RNA sequencing: Micro RNA sequencing strategy designed to capture post-transcriptional RNA elements and include non-coding functional elements',
    'ncRNA-Seq' => 'Non-coding RNA sequencing: Capture of other non-coding RNA types, including post-translation modification types such as snRNA (small nuclear RNA) or snoRNA (small nucleolar RNA), or expression regulation types such as siRNA (small interfering RNA) or piRNA/piwi/RNA(piwi-interacting RNA)',
    'FINISHING' => 'Sequencing intended to finish (close) gaps in existing coverage.',
    'CTS' => 'Concatenated Tag Sequencing.',
    'Tn-Seq' => 'Transposon RNA sequencing: Quantitatively determine fitness of bacterial genes based on how many times a purposely seeded transposon gets inserted into each gene of a colony after some time. Gene fitness determination through transposon seeding.',
    'VALIDATION' => 'Validation sequencing: CGHub special request. Independent experiment to re-evaluate putative variants. Micro RNA sequencing strategy designed to capturepost-transcriptional RNA elements and include non-coding functional elements.',
    'FAIRE-seq' => 'Formaldehyde Assisted Isolation of Regulatory Elements sequencing: Used for determining the sequences of those DNA regions in the genome associated with regulatory activity.',
    'SELEX' => 'Systematic Evolution of Ligands by EXponential enrichment sequencing: An assay involving progressive selection of oligonucleotides with certain affinities to target molecules.',
    'RIP-Seq' => 'RNA-immunoprecipitation sequencing: Direct sequencing of RNA immunoprecipitates (includes CLIP-Seq, HITS-CLIP and PAR-CLIP).',
    'ChIA-PET' => 'Chromatin Interaction Analysis by Paired-End Tag sequencing: Direct sequencing of proximity-ligated chromatin immunoprecipitates.',
    'RAD-Seq' => 'Restriction site Associated DNA sequencing: A method for sampling the genomes of multiple individuals in a population using next generation DNA sequencing.',
  );
  foreach ($strategies as $strategy => $description) {
    tripal_insert_cvterm(array(
      'name' => $strategy,
      'def' => $description,
      'cv_name' => 'experiment_strategy',
      'db_name' => 'tripal_sra',
    ));
  }

  // library source
  tripal_insert_cv(
    'experiment_source',
    'The library source specifies the type of source material that is being sequenced.'
  );

  $sources = array(
    'GENOMIC' => 'Genomic DNA (includes PCR products from genomic DNA).',
    'TRANSCRIPTOMIC' => 'Transcription products or non genomic DNA (EST, cDNA, RT-PCR, screened libraries).',
    'METAGENOMIC' => 'Mixed material from metagenome.', 
    'METATRANSCRIPTOMIC' => 'Transcription products from community targets',
    'SYNTHETIC' => 'Synthetic DNA.',
    'VIRAL RNA' => 'Viral RNA.',
    'OTHER' => 'Other, unspecified, or unknown library source material.', 
  );

  foreach ($sources as $source => $description) {
    tripal_insert_cvterm(array(
      'name' => $source,
      'def' => $description,
      'cv_name' => 'experiment_source',
      'db_name' => 'tripal_sra',
    ));
  }

  // library selection
  tripal_insert_cv(
    'experiment_selection',
    'Whether any method was used to select and/or enrich the material being sequenced.'
  );

  $selections = array(
    'RANDOM' => 'Random selection by shearing or other method',
    'PCR' => 'Source material was selected by designed primers',
    'RANDOM PCR' => 'Source material was selected by randomly generated primers',
    'RT-PCR' => 'Source material was selected by reverse transcription PCR',
    'HMPR' => 'Hypo-methylated partial restriction digest',
    'MF' => 'Methyl Filtrated',
    'CF-S' => 'Cot-filtered single/low-copy genomic DNA',
    'CF-M' => 'Cot-filtered moderately repetitive genomic DNA',
    'CF-H' => 'Cot-filtered highly repetitive genomic DNA',
    'CF-T' => 'Cot-filtered theoretical single-copy genomic DNA',
    'MDA' => 'Multiple displacement amplification',
    'MSLL' => 'Methylation Spanning Linking Library',
    'cDNA' => 'complementary DNA',
    'ChIP' => 'Chromatin immunoprecipitation',
    'MNase' => 'Micrococcal Nuclease (MNase) digestion',
    'DNAse' => 'Deoxyribonuclease (MNase) digestion',
    'Hybrid Selection' => 'Selection by hybridization in array or solution',
    'Reduced Representation' => 'Reproducible genomic subsets, often generated by restriction fragment size selection, containing a manageable number of loci to facilitate re-sampling',
    'Restriction Digest' => 'DNA fractionation using restriction enzymes',
    '5-methylcytidine antibody' => 'Selection of methylated DNA fragments using an antibody raised against 5-methylcytosine or 5-methylcytidine (m5C)',
    'MBD2 protein methyl-CpG binding domain' => 'Enrichment by methyl-CpG binding domain',
    'CAGE' => 'Cap-analysis gene expression',
    'RACE' => 'Rapid Amplification of cDNA Ends',
    'size fractionation' => 'Physical selection of size appropriate targets',
    'Padlock probes capture method' => 'Circularized oligonucleotide probes',
    'other' => 'Other library enrichment, screening, or selection process (please include additional info in the “design description”)',
    'unspecified' => 'Library enrichment, screening, or selection is not specified (please include additional info in the “design description”)',
  );

  foreach ($selections as $selection => $description) {
    tripal_insert_cvterm(array(
      'name' => $selection,
      'def' => $description,
      'cv_name' => 'experiment_selection',
      'db_name' => 'tripal_sra',
    ));
  }

  // libraray layout (without nominal size and nominal sd)
  tripal_insert_cv(
    'experiment_layout',
    'fragment, or paired.'
  );

  tripal_insert_cvterm(array(
    'name' => 'PAIRED',
    'def' => 'paired-end library',
    'cv_name' => 'experiment_layout',
    'db_name' => 'tripal_sra',
  ));

  tripal_insert_cvterm(array(
    'name' => 'FRAGMET',
    'def' => 'single-end library',
    'cv_name' => 'experiment_layout',
    'db_name' => 'tripal_sra',
  ));

  // other experiment property
  tripal_insert_cv(
    'experiment_property',
    'Contains property terms for experiment.'
  ); 

  $exp_props = array(
    'planned_read_length' => 'planned_read_length',
    'planned_read_length_mate_1' => 'planned read length for mate 1',
    'planned_read_length_mate_2' => 'planned read length for mate 2',
    'nominal_size' => 'Nominal size (bp)',
    'nominal_standard_deviation' => 'Nominal standard deviation (bp)',
    'jbrowse_data' => 'jbrowse data',
    'jbrowse_track' => 'jbrowse track',
    'total_count' => 'total number of reads after clean',
  );

  foreach ($exp_props as $exp_prop => $description) {
    tripal_insert_cvterm(array(
      'name' => $exp_prop,
      'def' => $description,
      'cv_name' => 'experiment_property',
      'db_name' => 'tripal_sra',
    ));
  }

  // Insert the basic biosample prop types. These types are used in the NCBI
  // BioSample database.

  // We use NCBI biosample attributes as to fill the biomaterialprop cv table.
  // These attributes can be accessed at the following url.
  $url = "http://www.ncbi.nlm.nih.gov/biosample/docs/attributes/?format=xml";
  $xml_str = file_get_contents($url);
  $xml_obj = simplexml_load_string($xml_str);

  foreach ($xml_obj->Attribute as $attribute) {
    tripal_insert_cvterm(array(
      'name' => $attribute->HarmonizedName,
      'def' => $attribute->Description,
      'cv_name' => 'biosample_property',
      'db_name' => 'tripal_sra',
    ));
  }
}

/**
 * Implements hook_uninstall().
 *
 * @ingroup tripal_sra
 */
function tripal_sra_uninstall() {
/*
  // Get localization function for installation.
  $t = get_t();
*/
}
