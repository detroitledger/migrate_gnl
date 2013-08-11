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
      'nodes' => array(
        'title' => t('Nodes'),
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
      'Orgs' => array(
        'class_name' => 'Orgs',
        'group_name' => 'nodes',
      ),
      'Grants' => array(
        'class_name' => 'Grants',
        'group_name' => 'nodes',
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
        "type" => array(
          'type' => 'varchar',
          'length' => '255',
          'not null' => TRUE,
          'description' => 'grant type',
        )
      ),
      MigrateDestinationTerm::getKeySchema()
    );
    $this->source = new MigrateSourceCSV('/tmp/migrate_gnl/classify_grants.csv', $this->csvcols(), array('header_rows'=>4,'embedded_newlines'=>FALSE));
    $this->destination = new MigrateDestinationTerm('grant_types');
    $this->addFieldMapping('name', 'type');
    $this->addFieldMapping('description', 'description');
  }
  protected function csvcols() {
    $cols[0] = array('type', 'grant type');
    $cols[1] = array('description', 'description');
    return $cols;
  }
}

class OrgNTEETypes extends Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = t('Import organization NTEE codes and descriptions from CSV into term name and ntee code fields.');
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        "code" => array(
          'type' => 'varchar',
          'length' => '8',
          'not null' => TRUE,
          'description' => 'ntee code',
        )
      ),
      MigrateDestinationTerm::getKeySchema()
    );
    $this->source = new MigrateSourceCSV('/tmp/migrate_gnl/classify_nonprofits_detail.csv', $this->csvcols(), array('header_rows'=>4,'embedded_newlines'=>FALSE));
    $this->destination = new MigrateDestinationTerm('ntee');
    $this->addFieldMapping('name', 'description');
    $this->addFieldMapping('field_ntee_code', 'code');
    $this->addFieldMapping('parent', 'parent_tid');
  }
  protected function csvcols() {
    $cols[0] = array('code', 'ntee code');
    $cols[1] = array('description', 'description');
    return $cols;
  }
  public function prepareRow($row) {
    if (strlen($row->code) > 1) { // e.g. A1 or A5E as opp. to C
                                  // A must come before A1, so we can look it up!
      $letter_tid = db_select('field_data_field_ntee_code', 'ntee')
                       ->fields('ntee', array('entity_id'))
                       ->condition('field_ntee_code_value', $row->code[0])
                       ->execute()
                       ->fetchField();
      if ($letter_tid) {
        $row->parent_tid = $letter_tid;
      }
    }
  }
}

class Orgs extends Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = t('Import organizations from a CSV like orgs_masterlist_NTEE.csv');
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        "ORGANIZATION_NAME" => array(
          'type' => 'varchar',
          'length' => '128',
          'not null' => TRUE,
          'description' => 'org name',
        )
      ),
      MigrateDestinationNode::getKeySchema()
    );
    $this->source = new MigrateSourceCSV('/tmp/migrate_gnl/orgs_masterlist_NTEE.csv', array(), array('header_rows'=>1, 'embedded_newlines'=>FALSE));
    $this->destination = new MigrateDestinationNode('org');
    $this->addFieldMapping('title', 'ORGANIZATION_NAME');
    $this->addFieldMapping(NULL, 'ORG_NTEE'); // used by prepareRow()
    $this->addFieldMapping('field_ntee', 'ntees');
    $this->addFieldMapping('field_ntee:source_type')->defaultValue('tid');
  }

  public function prepareRow($row) {
    $codes = array_map('trim', explode(';', $row->ORG_NTEE));
    $tids = array();
    foreach ($codes as $code) {
      $tids[] = db_select('field_data_field_ntee_code', 'ntee')
                  ->fields('ntee', array('entity_id'))
                  ->condition('field_ntee_code_value', $code)
                  ->execute()
                  ->fetchField();
    }
    $row->ntees = $tids;
  }

  public function fields() {
    return array(
      'title' => 'Organization name',
      'ntees' => 'Comma-separated list of NTEE tids',
    );
  }
}


class Grants extends Migration {

  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = t('Import grants from a CSV');

    // Set up the source
    $this->source = new MigrateSourceCSV('/tmp/migrate_gnl/allgrants.csv', array(), array('header_rows'=>1, 'embedded_newlines'=>FALSE));

    // Map fields in the source to the destination.
    // This defines the key fields for the source and the destination.
    // The first parameter names the map.
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'id' => array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'description' => 'id',
         ),
       ),
      MigrateDestinationNode::getKeySchema()
    );

    // Set up grants as the destination
    $this->destination = new MigrateDestinationNode('grant');


    // destination field is the first parameter
    // and the source field is the second parameter
    $this->addFieldMapping('title', 'title'); // should be auto generated...
    $this->addFieldMapping(NULL, 'granter'); // used by prepareRow()
    $this->addFieldMapping(NULL, 'grantee'); // used by prepareRow()
    $this->addFieldMapping(NULL, 'amount'); // used by prepareRow()
    $this->addFieldMapping(NULL, 'year'); // used by prepareRow()
    $this->addFieldMapping('field_source', 'data_source');
    $this->addFieldMapping('field_funder', 'granter_nid');
    $this->addFieldMapping('field_recipient', 'grantee_nid');
    $this->addFieldMapping('field_funded_amount', 'amount_clean');
    $this->addFieldMapping('field_year', 'years_clean');
    $this->addFieldMapping('body', 'notes');
    $this->addFieldMapping('field_internal_notes', 'internal_notes_blob');
  }

  public function getNidByOrgName($orgName) {
    $node = db_select('node', 'n')
              ->fields('n')
              ->condition('title', $orgName, '=')
              ->execute()
              ->fetch();
    $nid = $node->nid;
    return $nid;
  }

  public function prepareRow($row) {
    // Make a title
    $row->title = $row->granter . ' -> ' . $row->grantee;

    // Get the start year
    $year_raw = $row->year;
    $year_start = intval($year_raw);

    // Get the end year
    $length = intval($row->length_years);
    $year_end = $year_start + $length;

    $date_data = array(
      'from' => (string) '01/01/' . $year_start . ' 00:00:00',
      'to' => (string) '01/01/' . $year_end . ' 00:00:00',
    );

    $row->years_clean = drupal_json_encode($date_data);

    // Get the organziation's IDs
    $row->granter_nid = $this->getNidByOrgName($row->granter);
    $row->grantee_nid = $this->getNidByOrgName($row->grantee);

    $row->amount_clean = (int) str_replace('$', '', str_replace(' ', '', str_replace(',', '', $row->amount)));

    // store extra infos as JSON in internal_notes
    if (strlen($row->impact_area) > 1) {
      $internal_noteses['impact_area'] = $row->impact_area;
    }
    if (strlen($row->impact_neighborhood) > 1) {
      $internal_noteses['impact_neighborhood'] = $row->impact_neighborhood;
    }
    $row->internal_notes_blob = drupal_json_encode($internal_noteses);
  }


}
