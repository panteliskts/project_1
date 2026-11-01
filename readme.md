# Αναζήτηση εικόνων με διανυσματικές αναπαραστάσεις

1η Προγραμματιστική Εργασία, Κ23γ: Ανάπτυξη Λογισμικού για Αλγοριθμικά Προβλήματα, χειμερινό εξάμηνο 2026-27.

- Παντελής Κώτσας, sdi2300239
- Σινάν Κιουτσουκ, sdi2300074

Κάθε εικόνα γίνεται ένα διάνυσμα $x_I \in \mathbb{R}^K$ (Bag of Visual Words): εξάγουμε SIFT descriptors με OpenCV, μαθαίνουμε $K$ κέντρα με k-means πάνω σε descriptors του training set του HPatches και αναπαριστούμε την εικόνα με το ποσοστό των descriptors της που πέφτουν σε κάθε κέντρο. Πάνω σε αυτά τα διανύσματα τρέχουμε πέντε μεθόδους αναζήτησης (Exact, LSH, Hypercube, IVFFlat, IVFPQ) με ευκλείδεια απόσταση και μετράμε Recall@5, Recall@10, mAP@10 και χρόνο ανά query.

## Δομή του κώδικα

Ο κώδικας χωρίζεται σε δύο ανεξάρτητα μέρη. Ο αλγοριθμικός πυρήνας (`core/`) δεν ξέρει τίποτα για εικόνες ή OpenCV: παίρνει πίνακα διανυσμάτων και απαντά σε ερωτήματα κοντινότερων γειτόνων. Το pipeline (`pipeline/`) φτιάχνει τα διανύσματα από τις εικόνες, διαβάζει τις παραμέτρους, αξιολογεί και γράφει την έξοδο. Οι επικεφαλίδες (`.hpp`) βρίσκονται στο `include/` και οι υλοποιήσεις (`.cpp`) στο `src/`, με τους ίδιους υποφακέλους ανά ομάδα αρχείων (π.χ. `include/core/lsh/lsh.hpp` και `src/core/lsh/lsh.cpp`, μαζί με το Hypercube και τις προβολές). Τα `#include` γράφονται σχετικά με το `include/` (π.χ. `#include "core/common/vec.hpp"`), και ο πίνακας παρακάτω δίνει τις διαδρομές χωρίς το `include/` ή `src/` μπροστά.

| Αρχείο | Περιεχόμενο |
| --- | --- |
| `core/common/vec.hpp/.cpp` | `Dataset` (n διανύσματα διάστασης dim σε συνεχόμενη μνήμη), `Neighbor`, ευκλείδεια απόσταση, επιλογή top-N |
| `core/common/random.hpp` | γεννήτρια τυχαίων αριθμών πάνω στο `std::mt19937_64` (ίδια αποτελέσματα σε Linux και macOS για το ίδιο seed) |
| `core/common/parallel.hpp` | `parallelFor` με `std::thread` |
| `core/common/search_index.hpp` | αφηρημένη κλάση `SearchIndex` (`build`, `search`) που υλοποιούν όλες οι μέθοδοι |
| `core/clustering/kmeans.hpp/.cpp` | k-means (Lloyd) με αρχικοποίηση k-means++, κοινό για λεξιλόγιο, IVF και PQ |
| `core/exact/exact.hpp/.cpp` | εξαντλητική αναζήτηση |
| `core/lsh/projection.hpp/.cpp` | συναρτήσεις $h(p) = \lfloor (v \cdot p + t) / w \rfloor$, κοινές για LSH και Hypercube |
| `core/lsh/lsh.hpp/.cpp` | LSH με L πίνακες κατακερματισμού |
| `core/lsh/hypercube.hpp/.cpp` | τυχαία προβολή σε υπερκύβο |
| `core/ivf/ivf.hpp/.cpp` | coarse quantizer (k-means + inverted lists), κοινός για IVFFlat και IVFPQ |
| `core/ivf/ivfflat.hpp/.cpp` | IVFFlat |
| `core/ivf/ivfpq.hpp/.cpp` | IVFPQ με product quantization των residuals και ADC |
| `pipeline/cli/cli.hpp/.cpp` | ανάγνωση και έλεγχος παραμέτρων γραμμής εντολών |
| `pipeline/dataset/split.hpp/.cpp` | ανάγνωση του `split.txt` |
| `pipeline/dataset/database.hpp/.cpp` | κατασκευή queries και βάσης για το validation ή το test set |
| `pipeline/features/features.hpp/.cpp` | SIFT (OpenCV), εκπαίδευση λεξιλογίου, διάνυσμα $x_I$ |
| `pipeline/features/cache.hpp/.cpp` | αποθήκευση και φόρτωση λεξιλογίου και διανυσμάτων σε δυαδικά αρχεία |
| `pipeline/evaluation/metrics.hpp/.cpp` | Recall@5, Recall@10, AP@10, μέσοι όροι |
| `pipeline/evaluation/output.hpp/.cpp` | αρχείο εξόδου στη μορφή της εκφώνησης |
| `src/main.cpp` | ενώνει τα παραπάνω |
| `tests/test_core.cpp` | έλεγχοι του πυρήνα σε συνθετικά δεδομένα (χωρίς εικόνες) |
| `scripts/experiments.py` | εκτέλεση της πειραματικής μελέτης (μόνο standard library της Python) |
| `split.txt` | διαχωρισμός train / validation / test |

## Μεταγλώττιση

Χρειάζεται compiler με C++17, `make`, `pkg-config` και OpenCV 4.4 ή νεότερη (εκεί μπήκε το SIFT στο κύριο module). Το Makefile βρίσκει μόνο του αν είναι εγκατεστημένη η OpenCV 4 (`opencv4.pc`) ή η 5 (`opencv5.pc`).

```bash
sudo apt install build-essential pkg-config libopencv-dev   # Ubuntu / Debian
make            # φτιάχνει το ./search
make test       # μεταγλωττίζει και τρέχει τους ελέγχους του πυρήνα (δεν θέλει OpenCV)
make clean
```

## Δεδομένα

Τα links της εκφώνησης δεν δουλεύουν πια (το `icvl.ee.ic.ac.uk` δεν απαντά και το `mirflickr25k.zip` δίνει 404). Τα ίδια αρχεία υπάρχουν εδώ:

```bash
mkdir -p data/hpatches data/mirflickr
# HPatches full sequences (1.3 GB), επίσημο mirror από το README του hpatches-dataset
curl -L -o data/hpatches/hpatches-sequences-release.zip \
  https://huggingface.co/datasets/vbalnt/hpatches/resolve/main/hpatches-sequences-release.zip
unzip -q data/hpatches/hpatches-sequences-release.zip -d data/hpatches
# MIRFlickr-25K (2.9 GB)
curl -L -o data/mirflickr/mirflickr25k.zip \
  https://press.liacs.nl/mirflickr/mirflickr25k.v3b/mirflickr25k.zip
unzip -q data/mirflickr/mirflickr25k.zip -d data/mirflickr
```

Μετά από αυτό, `-hp data/hpatches/hpatches-sequences-release` και `-mir data/mirflickr/mirflickr`.

## Εκτέλεση

```bash
./search -hp data/hpatches/hpatches-sequences-release -mir data/mirflickr/mirflickr \
  -split split.txt -vocab 256 -S 500 -D 10000 \
  -set validation -o output.txt \
  -ivfflat -kclusters 100 -nprobe 10 -seed 1
```

Οι παράμετροι δίνονται με οποιαδήποτε σειρά. Εκτός από το αρχείο εξόδου, το πρόγραμμα τυπώνει στην οθόνη τους μέσους όρους συνολικά και χωριστά για illumination και viewpoint, τον χρόνο κατασκευής του ευρετηρίου και πόσα queries πήραν λιγότερα από 10 αποτελέσματα.

### Κοινές παράμετροι

| Παράμετρος | Σημασία | Default |
| --- | --- | --- |
| `-hp <path>` | κατάλογος HPatches (περιέχει τους καταλόγους `i_*`, `v_*`) | υποχρεωτική |
| `-mir <path>` | κατάλογος MIRFlickr (περιέχει τα `im*.jpg`) | υποχρεωτική αν D > 0 |
| `-split <file>` | αρχείο διαχωρισμού | υποχρεωτική |
| `-vocab <int>` | πλήθος K κέντρων του λεξιλογίου | 256 |
| `-S <int>` | μέγιστο πλήθος descriptors ανά training εικόνα για το k-means | 500 |
| `-D <int>` | πλήθος εικόνων MIRFlickr στη βάση | 1000 |
| `-set <validation\|test>` | σύνολο queries | υποχρεωτική |
| `-o <file>` | αρχείο εξόδου | υποχρεωτική |
| `-seed <int>` | seed για όλες τις τυχαίες διαδικασίες | 1 |
| `-cache <dir>` | (επιπλέον) κατάλογος για το λεξιλόγιο και τα διανύσματα, βλ. παρακάτω | χωρίς cache |

Μέθοδος (ακριβώς μία): `-exact`, `-lsh`, `-hypercube`, `-ivfflat`, `-ivfpq`.

### Παράμετροι μεθόδων

| Μέθοδος | Παράμετρος | Σημασία | Default |
| --- | --- | --- | --- |
| LSH | `-k` | πλήθος $h_i$ σε κάθε $g$ | 4 |
| | `-L` | πλήθος πινάκων κατακερματισμού | 5 |
| | `-w` | μήκος κελιού στην ευθεία προβολής | 0.05 |
| Hypercube | `-kproj` | διάσταση $d'$ του υπερκύβου (έως 30) | 10 |
| | `-w` | μήκος κελιού | 0.05 |
| | `-M` | μέγιστο πλήθος υποψηφίων που ελέγχονται | 500 |
| | `-probes` | μέγιστο πλήθος κορυφών που ελέγχονται | 10 |
| IVFFlat | `-kclusters` | πλήθος συστάδων | $\lceil\sqrt{n}\rceil$ |
| | `-nprobe` | συστάδες που ελέγχονται | 5 |
| IVFPQ | `-kclusters` | πλήθος συστάδων | $\lceil\sqrt{n}\rceil$ |
| | `-nprobe` | συστάδες που ελέγχονται | 5 |
| | `-M` | πλήθος υποδιανυσμάτων, πρέπει να διαιρεί το K | 16 |
| | `-nbits` | $2^{nbits}$ κέντρα ανά υποχώρο (1 έως 16) | 8 |

### Cache

Η εξαγωγή SIFT για 25000 εικόνες παίρνει μερικά λεπτά, οπότε με `-cache <dir>` το λεξιλόγιο και τα διανύσματα $x_I$ αποθηκεύονται σε δυαδικά αρχεία στον κατάλογο `<dir>` και ξαναχρησιμοποιούνται στις επόμενες εκτελέσεις. Χωρίς την παράμετρο το πρόγραμμα υπολογίζει τα πάντα από την αρχή και δεν γράφει τίποτα στον δίσκο εκτός από το αρχείο εξόδου.

Ένα αρχείο του cache ξαναχρησιμοποιείται μόνο αν ταιριάζουν όλα όσα το επηρεάζουν. Το όνομά του περιέχει τα K, S, seed, ένα hash του περιεχομένου του `split.txt` και ένα hash της απόλυτης διαδρομής του καταλόγου HPatches, και για τα διανύσματα του MIRFlickr επιπλέον ένα hash της διαδρομής του καταλόγου MIRFlickr. Αν αλλάξει κάποιο από αυτά, το αρχείο δεν βρίσκεται και όλα υπολογίζονται ξανά. Για το MIRFlickr κρατάμε όσες εικόνες έχουν ήδη υπολογιστεί, και ένα μεγαλύτερο D επεξεργάζεται μόνο τις καινούργιες. Αν αλλάξουν οι ίδιες οι εικόνες μέσα στους καταλόγους, σβήνουμε τον κατάλογο του cache. Το script των πειραμάτων τρέχει πάντα με `-cache cache`. Το cache δεν επηρεάζει τον χρόνο αναζήτησης, ο οποίος μετράει μόνο την κλήση `search()` για κάθε query.

### Παράλληλα τμήματα

Παράλληλα, με δικά μας νήματα (`parallelFor` στο `include/core/common/parallel.hpp`, πάνω στο `std::thread`), τρέχουν μόνο βήματα που δεν μετράνε στον χρόνο αναζήτησης:

- **Εξαγωγή SIFT και διανύσματα $x_I$.** Μία εικόνα ανά νήμα. Για τις εικόνες του HPatches χρησιμοποιούμε το πολύ 2 νήματα, γιατί το SIFT στις μεγαλύτερες (3088x2056) θέλει περίπου 1.5 GB μνήμης η καθεμία και με ένα νήμα ανά πυρήνα η μνήμη έφτανε τα 7 GB. Οι εικόνες του MIRFlickr είναι μικρές και χρησιμοποιούν όλους τους πυρήνες. Τα εσωτερικά νήματα της OpenCV είναι κλειστά.
- **k-means** (λεξιλόγιο και κατασκευή των ευρετηρίων IVFFlat και IVFPQ). Τα περάσματα πάνω σε όλα τα σημεία (αρχικοποίηση k-means++ και ανάθεση στο πλησιέστερο κέντρο) γίνονται παράλληλα, εκτός από τα πολύ μικρά, που τρέχουν σειριακά.

Η αναζήτηση είναι σειριακή για όλες τις μεθόδους: τα queries εκτελούνται ένα ένα σε ένα νήμα και καμία μέθοδος δεν χρησιμοποιεί νήματα μέσα στη `search()`, ώστε οι χρόνοι να είναι συγκρίσιμοι.

## Έξοδος

Το αρχείο εξόδου έχει ακριβώς τη μορφή της εκφώνησης. Το όνομα της μεθόδου στην πρώτη γραμμή είναι `Exact Search`, `LSH`, `Hypercube`, `IVFFlat` ή `IVFPQ`. Αν μια προσεγγιστική μέθοδος βρει λιγότερους από 10 υποψηφίους, οι θέσεις που λείπουν γράφονται `Rank-k: -` και μετράνε ως μη σχετικές. Ο χρόνος δίνεται σε milliseconds.

## Σημειώσεις υλοποίησης

- **Λεξιλόγιο.** Από κάθε μία από τις 420 εικόνες του training set (70 ακολουθίες επί 6) διαλέγουμε τυχαία έως S descriptors με partial Fisher-Yates, και το k-means τρέχει στο σύνολό τους. Οι εικόνες MIRFlickr και validation/test δεν χρησιμοποιούνται στην εκπαίδευση.
- **MIRFlickr.** Τα αρχεία ταξινομούνται κατά τον αριθμό τους (`im1.jpg`, `im2.jpg`, ...). Όσα δεν δίνουν κανέναν descriptor παραλείπονται και παίρνουμε το επόμενο.
- **LSH.** Κάθε πίνακας χρησιμοποιεί σαν κλειδί ολόκληρο το $ID(p) = \sum_i r_i h_i(p) \bmod 2^{64}$, άρα σε έναν κάδο μπαίνουν μόνο σημεία με ίδιες όλες τις k τιμές $h_i$. Οι υποψήφιοι είναι η ένωση των L κάδων του query και ταξινομούνται με την πραγματική απόσταση.
- **Hypercube.** Το bit j της κορυφής είναι $f_j(h_j(p))$, όπου η $f_j$ είναι ένα ντετερμινιστικό hash του $(j, h_j(p))$ με τιμές στο $\{0,1\}$. Έτσι μια τιμή $h_j$ που εμφανίζεται πρώτη φορά στο query παίρνει πάντα το ίδιο bit και δεν χρειάζεται πίνακας. Οι κορυφές εξετάζονται κατά αύξουσα απόσταση Hamming (πρώτα η κορυφή του query) μέχρι να ελεγχθούν `probes` κορυφές ή `M` σημεία.
- **IVFFlat / IVFPQ.** Το k-means του ευρετηρίου εκπαιδεύεται σε όλα τα n διανύσματα της βάσης. Στο IVFPQ κβαντίζονται τα residuals $x - c(x)$ και η απόσταση υπολογίζεται με ADC: για κάθε συστάδα που ελέγχεται φτιάχνουμε πίνακα αποστάσεων $M \times 2^{nbits}$ από τα υποδιανύσματα του residual του query, οπότε η απόσταση από κάθε αποθηκευμένο κωδικό είναι άθροισμα M τιμών του πίνακα. Η κατάταξη γίνεται με αυτή την προσεγγιστική απόσταση, χωρίς re-ranking.
- **Αναπαραγωγιμότητα.** Όλα τα τυχαία βήματα ξεκινούν από το `-seed`. Οι κατανομές `std::normal_distribution` κλπ. δίνουν διαφορετικούς αριθμούς σε libstdc++ και libc++, γι' αυτό ο πυρήνας παράγει μόνος του ομοιόμορφες και κανονικές τιμές από το `std::mt19937_64`.

## Πειραματική μελέτη

```bash
python3 scripts/experiments.py vocab            # επιλογή K με Exact στο validation, D = 1000
python3 scripts/experiments.py sweep -vocab 256 # πλέγματα παραμέτρων στο validation για κάθε D
python3 scripts/experiments.py test  -vocab 256 # οι επιλεγμένες παράμετροι στο test set (διάμεσος χρόνος 3 εκτελέσεων)
python3 scripts/experiments.py report -vocab 256 # πίνακες και δεδομένα γραφημάτων για την αναφορά
```

Τα αποτελέσματα γράφονται στο `results/` (`vocab.csv`, `validation.csv`, `selected.csv`, `test.csv`, ενώ τα αρχεία εξόδου κάθε εκτέλεσης μένουν στο `results/runs/`, που δεν μπαίνει στο git). Για κάθε μέθοδο και κάθε D διαλέγουμε στο validation τη γρηγορότερη ρύθμιση που φτάνει τουλάχιστον το 95% του mAP@10 της Exact Search, ή τη ρύθμιση με το μεγαλύτερο mAP@10 αν καμία δεν φτάνει εκεί. Το K = 256 βγήκε από το στάδιο `vocab`: είναι το μικρότερο K με mAP@10 το πολύ 0.005 κάτω από το καλύτερο.

Η αναφορά είναι το `report/report.pdf` (πρότυπο IEEE δύο στηλών). Οι πίνακες και τα γραφήματα διαβάζουν τα αρχεία του `report/generated/`, που παράγει το στάδιο `report`. Μεταγλωττίζεται με XeLaTeX, π.χ. `cd report && tectonic report.tex`, γιατί χρησιμοποιεί τη γραμματοσειρά Times New Roman μέσω fontspec για ελληνικά και λατινικά.
