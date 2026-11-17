-- =====================================================================
--  PET MANAGER - AJOUTS aux tables de la base commune (base_commune_petmanager.sql
--  reste INCHANGE : on ne fait ici que des ALTER TABLE).
--  A executer UNE FOIS, connecte en SYSTEM, "Executer le script" (F5).
--  Peut etre relance sans risque (rien n'est fait si c'est deja la).
--
--  Ce que les modules de l'application utilisent en plus du modele papier :
--    ANIMAL     : STATUT, IDPERE, IDMERE (arbre genealogique)
--    COMMANDE   : QUANTITE
--    COMMANDE.IDEMPLOYE et RENDEZVOUS.IDEMPLOYE : doivent accepter NULL
--                 (l'interface ne demande pas l'employe)
-- =====================================================================
ALTER SESSION SET CURRENT_SCHEMA = PETMANAGER;

DECLARE
  PROCEDURE colonne(tab VARCHAR2, col VARCHAR2, ddl VARCHAR2) IS
    n NUMBER;
  BEGIN
    SELECT COUNT(*) INTO n FROM all_tab_columns
     WHERE owner = 'PETMANAGER' AND table_name = tab AND column_name = col;
    IF n = 0 THEN EXECUTE IMMEDIATE 'ALTER TABLE ' || tab || ' ADD (' || col || ' ' || ddl || ')'; END IF;
  END;
  PROCEDURE contrainte(tab VARCHAR2, nom VARCHAR2, ddl VARCHAR2) IS
    n NUMBER;
  BEGIN
    SELECT COUNT(*) INTO n FROM all_constraints
     WHERE owner = 'PETMANAGER' AND table_name = tab AND constraint_name = nom;
    IF n = 0 THEN EXECUTE IMMEDIATE 'ALTER TABLE ' || tab || ' ADD CONSTRAINT ' || nom || ' ' || ddl; END IF;
  END;
  PROCEDURE nullable(tab VARCHAR2, col VARCHAR2) IS
  BEGIN
    EXECUTE IMMEDIATE 'ALTER TABLE ' || tab || ' MODIFY (' || col || ' NULL)';
  EXCEPTION WHEN OTHERS THEN
    IF SQLCODE != -1451 THEN RAISE; END IF;      -- ORA-01451 : deja nullable
  END;
BEGIN
  -- ANIMAL
  colonne('ANIMAL', 'STATUT', 'VARCHAR2(20)');
  colonne('ANIMAL', 'IDPERE', 'NUMBER(8)');
  colonne('ANIMAL', 'IDMERE', 'NUMBER(8)');
  contrainte('ANIMAL', 'CK_ANIMAL_STATUT',
             'CHECK (STATUT IS NULL OR STATUT = ''Surveillance'' OR STATUT LIKE ''En bonne sant_'')');
  contrainte('ANIMAL', 'FK_ANIMAL_PERE', 'FOREIGN KEY (IDPERE) REFERENCES ANIMAL (IDANIMAL) ON DELETE SET NULL');
  contrainte('ANIMAL', 'FK_ANIMAL_MERE', 'FOREIGN KEY (IDMERE) REFERENCES ANIMAL (IDANIMAL) ON DELETE SET NULL');
  contrainte('ANIMAL', 'CK_ANIMAL_PAS_SOI',
             'CHECK ((IDPERE IS NULL OR IDPERE <> IDANIMAL) AND (IDMERE IS NULL OR IDMERE <> IDANIMAL))');
  contrainte('ANIMAL', 'CK_ANIMAL_PERE_MERE', 'CHECK (IDPERE IS NULL OR IDMERE IS NULL OR IDPERE <> IDMERE)');

  -- COMMANDE
  colonne('COMMANDE', 'QUANTITE', 'NUMBER(8)');
  contrainte('COMMANDE', 'CK_CMD_QUANTITE', 'CHECK (QUANTITE IS NULL OR QUANTITE > 0)');
  nullable('COMMANDE', 'IDEMPLOYE');

  -- RENDEZVOUS
  nullable('RENDEZVOUS', 'IDEMPLOYE');
END;
/

-- Verification
SELECT table_name, column_name, nullable FROM all_tab_columns
 WHERE owner = 'PETMANAGER'
   AND ((table_name = 'ANIMAL'     AND column_name IN ('STATUT', 'IDPERE', 'IDMERE'))
     OR (table_name = 'COMMANDE'   AND column_name IN ('QUANTITE', 'IDEMPLOYE'))
     OR (table_name = 'RENDEZVOUS' AND column_name = 'IDEMPLOYE'))
 ORDER BY table_name, column_name;
