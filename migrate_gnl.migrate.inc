<?php

/**
 * @file migrate_gnl.migrate.inc
 * Migrate content from CSVs into detroit ledger taxonomy terms
 */

/**
 * Implements hook_migrate_api().
 */
function migrate_gnl_migrate_api() {
  $api = array(
    'api' => 2,
    'groups' => array(
      'terms' => array(
        'title' => t('Terms'),
      ),
    ),
    'migrations' => array(
      'GrantTypes' => array(
        'class_name' => 'GrantTypes',
        'group_name' => 'terms',
      ),
      'OrgNTEETypes' => array(
        'class_name' => 'OrgNTEETypes',
        'group_name' => 'terms',
      ),
    ),
  );
  return $api;
}

class GrantTypes extends Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = t('Import grant types from CSV.');
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        "TYPE" => array(
          'type' => 'varchar',
          'length' => '255',
          'not null' => TRUE,
          'description' => 'grant type',
        )
      ),
      MigrateDestinationTerm::getKeySchema()
    );
    $this->source = new MigrateSourceCSV('sites/default/files/csvs/classify_grants.csv', array(), array('header_rows'=>4,'embedded_newlines'=>FALSE));
    $this->destination = new MigrateDestinationTerm('grant_types');
    $this->addFieldMapping('name', 'TYPE');
    $this->addFieldMapping('description', 'DESCRIPTION');
  }
}

class OrgNTEETypes extends Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = t('Import organization NTEE codes and descriptions from CSV into term name and ntee code fields.');
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        "CODE" => array(
          'type' => 'varchar',
          'length' => '8',
          'not null' => TRUE,
          'description' => 'ntee code',
        )
      ),
      MigrateDestinationTerm::getKeySchema()
    );
    $this->source = new MigrateSourceCSV('sites/default/files/csvs/classify_nonprofits_detail.csv', array(), array('header_rows'=>4,'embedded_newlines'=>FALSE));
    $this->destination = new MigrateDestinationTerm('ntee');
    $this->addFieldMapping('name', 'DESCRIPTION');
    $this->addFieldMapping('field_ntee_code', 'CODE');
  }
}

