# Codexion — Todo list (15 jours)

| Jour | Objectif | Durée estimée |
|---|---|---|
| **1**   | | Structure du repo + Makefile (NAME, all, clean, fclean, re, flags) | 2-3h |
| **2**   | | Parsing des 8 arguments (récupération brute, pas de validation encore) | 2-3h |
| **3**   | | Validation stricte des arguments (entiers positifs, scheduler = fifo/edf exact, messages d'erreur) | 3h |
| **4**   | | Design des structs (`coder_t`, `dongle_t`, `simulation_t`) — zéro globale | 3h |
| **5**   | | Fonctions utilitaires de temps (timestamp ms, sleep précis) | 2h |
| **6**   | | Système de log thread-safe (mutex sur l'output, format imposé) | 2-3h |
| **7**   | | Création des threads coders + lifecycle basique (create/join, boucle vide) | 3h |
| **8**   | | Mutex par dongle + acquisition/relâchement basique (sans scheduler ni cooldown) | 3-4h |
| **9**   | | Cooldown des dongles (indisponibilité après relâchement) | 2-3h |
| **10**  | | Priority queue/heap maison + scheduler FIFO | 3-4h |
| **11**  | | Scheduler EDF (deadline = last_compile_start + time_to_burnout) + tie-breaker | 3-4h |
| **12**  | | Intégration machine à états complète (compile → debug → refactor → repeat) | 3-4h |
| **13**  | | Thread monitor + détection burnout précise (<10ms) + arrêt propre de la simu | 4h |
| **14**  | | Edge cases (1 seul coder, stress tests, valgrind, norminette) + fix des bugs | 3-4h |
| **15**  | | README complet (contenu réel) + tests finaux + relecture générale | 2-3h |

**Total estimé : ~42-48h**

## Notes
- Théorie complète du sujet (analogie philosophes, cycle de vie coder, dongles, scheduler) déjà expliquée en conversation — à relire avant le Jour 4 (design des structs) au plus tard, parce que là ça devient nécessaire.
- Deadlock comprehension check : à faire avant d'attaquer le Jour 8 (dongles).