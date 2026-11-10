-- NOTE ENCODAGE : les textes accentués sont écrits avec UNISTR('\00E9' = é ...)
-- =====================================================================
--  PET MANAGER - Données de test du MODULE ANIMAUX
--  À exécuter APRÈS base_commune_petmanager.sql (table ANIMAL avec colonne STATUT).
--  À exécuter une seule fois (un deuxième lancement donne ORA-00001 : identifiant déjà existant).
-- =====================================================================
INSERT INTO ANIMAL (IDANIMAL, PHOTO, NOM, ESPECE, RACE, SEXE, DATENAISSANCE, POIDS, DESCRIPTION, STATUT) VALUES (1, NULL, 'Rex',  'Chien', 'Berger allemand', UNISTR('M\00E2le'), ADD_MONTHS(TRUNC(SYSDATE), -48), 32.5, UNISTR('Vaccins \00E0 jour'), 'En bonne sant' || UNISTR('\00E9'));
INSERT INTO ANIMAL (IDANIMAL, PHOTO, NOM, ESPECE, RACE, SEXE, DATENAISSANCE, POIDS, DESCRIPTION, STATUT) VALUES (2, NULL, 'Mimi', 'Chat',  'Persan',          'Femelle',          ADD_MONTHS(TRUNC(SYSDATE), -30), 4.2,  'Allergie alimentaire', 'Surveillance');
INSERT INTO ANIMAL (IDANIMAL, PHOTO, NOM, ESPECE, RACE, SEXE, DATENAISSANCE, POIDS, DESCRIPTION, STATUT) VALUES (3, NULL, 'Max',  'Chien', 'Labrador',        UNISTR('M\00E2le'), ADD_MONTHS(TRUNC(SYSDATE), -14), 24.0, 'Suivi de croissance', 'En bonne sant' || UNISTR('\00E9'));
COMMIT;
