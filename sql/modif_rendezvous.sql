-- =====================================================================
--  Module Rendez-vous : adaptation de la table RENDEZVOUS
--  Le formulaire Rendez-vous ne demande pas encore l'employé (le module
--  Employés n'est pas encore intégré) : IDEMPLOYE doit accepter NULL.
--  À exécuter UNE FOIS, après base_commune_petmanager.sql.
-- =====================================================================
ALTER TABLE RENDEZVOUS MODIFY (IDEMPLOYE NULL);
