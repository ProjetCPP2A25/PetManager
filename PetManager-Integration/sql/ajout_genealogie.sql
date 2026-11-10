-- =====================================================================
--  PET MANAGER - AJOUT GENEALOGIE (arbre genealogique + consanguinite)
--  A executer UNE FOIS dans SQL Developer, connecte en tant que SYSTEM,
--  avec "Executer le script" (F5).  Peut etre relance sans risque.
--
--  Ajoute dans la table ANIMAL :
--    IDPERE, IDMERE  -> references vers un autre animal (NULL = inconnu)
--  ATTENTION : si l'equipe relance base_commune_petmanager.sql (qui recree
--  toutes les tables), il faut relancer ce script ensuite.
-- =====================================================================
ALTER SESSION SET CURRENT_SCHEMA = PETMANAGER;

-- 1) colonnes
DECLARE
  n NUMBER;
BEGIN
  SELECT COUNT(*) INTO n FROM all_tab_columns
   WHERE owner = 'PETMANAGER' AND table_name = 'ANIMAL' AND column_name = 'IDPERE';
  IF n = 0 THEN
    EXECUTE IMMEDIATE 'ALTER TABLE ANIMAL ADD (IDPERE NUMBER(8), IDMERE NUMBER(8))';
  END IF;
END;
/

-- 2) contraintes (cle etrangere vers la meme table, pas son propre parent, pere <> mere)
DECLARE
  PROCEDURE ajoute(nom VARCHAR2, ddl VARCHAR2) IS
    n NUMBER;
  BEGIN
    SELECT COUNT(*) INTO n FROM all_constraints
     WHERE owner = 'PETMANAGER' AND table_name = 'ANIMAL' AND constraint_name = nom;
    IF n = 0 THEN EXECUTE IMMEDIATE 'ALTER TABLE ANIMAL ADD CONSTRAINT ' || nom || ' ' || ddl; END IF;
  END;
BEGIN
  ajoute('FK_ANIMAL_PERE', 'FOREIGN KEY (IDPERE) REFERENCES ANIMAL (IDANIMAL) ON DELETE SET NULL');
  ajoute('FK_ANIMAL_MERE', 'FOREIGN KEY (IDMERE) REFERENCES ANIMAL (IDANIMAL) ON DELETE SET NULL');
  ajoute('CK_ANIMAL_PAS_SOI', 'CHECK ((IDPERE IS NULL OR IDPERE <> IDANIMAL) AND (IDMERE IS NULL OR IDMERE <> IDANIMAL))');
  ajoute('CK_ANIMAL_PERE_MERE', 'CHECK (IDPERE IS NULL OR IDMERE IS NULL OR IDPERE <> IDMERE)');
END;
/

-- 3) DONNEES DE DEMONSTRATION (ids 901 a 907, sans effet sur vos animaux)
--    Rocky (905) = petit de deux freres/soeurs  -> consanguinite 25 %
--    Rex   (1)   = parents Zeus x Kira          -> 0 % (arbre sur 3 generations)
DELETE FROM ANIMAL WHERE IDANIMAL BETWEEN 901 AND 907;

INSERT INTO ANIMAL (IDANIMAL, NOM, ESPECE, RACE, SEXE, DATENAISSANCE, POIDS, DESCRIPTION, STATUT) VALUES
  (901, 'Atlas', 'Chien', 'Berger allemand', UNISTR('M\00E2le'), ADD_MONTHS(TRUNC(SYSDATE), -120), 34.0, 'Fondateur', 'En bonne sant' || UNISTR('\00E9'));
INSERT INTO ANIMAL (IDANIMAL, NOM, ESPECE, RACE, SEXE, DATENAISSANCE, POIDS, DESCRIPTION, STATUT) VALUES
  (902, 'Bella', 'Chien', 'Berger allemand', 'Femelle', ADD_MONTHS(TRUNC(SYSDATE), -120), 28.0, 'Fondatrice', 'En bonne sant' || UNISTR('\00E9'));
INSERT INTO ANIMAL (IDANIMAL, NOM, ESPECE, RACE, SEXE, DATENAISSANCE, POIDS, DESCRIPTION, STATUT, IDPERE, IDMERE) VALUES
  (903, 'Zeus', 'Chien', 'Berger allemand', UNISTR('M\00E2le'), ADD_MONTHS(TRUNC(SYSDATE), -84), 33.0, 'Fils d''Atlas et Bella', 'En bonne sant' || UNISTR('\00E9'), 901, 902);
INSERT INTO ANIMAL (IDANIMAL, NOM, ESPECE, RACE, SEXE, DATENAISSANCE, POIDS, DESCRIPTION, STATUT, IDPERE, IDMERE) VALUES
  (904, 'Nala', 'Chien', 'Berger allemand', 'Femelle', ADD_MONTHS(TRUNC(SYSDATE), -84), 27.0, 'Fille d''Atlas et Bella', 'En bonne sant' || UNISTR('\00E9'), 901, 902);
INSERT INTO ANIMAL (IDANIMAL, NOM, ESPECE, RACE, SEXE, DATENAISSANCE, POIDS, DESCRIPTION, STATUT, IDPERE, IDMERE) VALUES
  (905, 'Rocky', 'Chien', 'Berger allemand', UNISTR('M\00E2le'), ADD_MONTHS(TRUNC(SYSDATE), -36), 31.0, 'Petit de Zeus et Nala', 'Surveillance', 903, 904);
INSERT INTO ANIMAL (IDANIMAL, NOM, ESPECE, RACE, SEXE, DATENAISSANCE, POIDS, DESCRIPTION, STATUT) VALUES
  (906, 'Kira', 'Chien', 'Berger allemand', 'Femelle', ADD_MONTHS(TRUNC(SYSDATE), -60), 26.5, 'Sans parents connus', 'En bonne sant' || UNISTR('\00E9'));

-- Rex (id 1) : parents Zeus (903) x Kira (906)
UPDATE ANIMAL SET IDPERE = 903, IDMERE = 906 WHERE IDANIMAL = 1;

COMMIT;

SELECT IDANIMAL, NOM, SEXE, IDPERE, IDMERE FROM ANIMAL ORDER BY IDANIMAL;
