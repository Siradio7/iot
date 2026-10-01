// Vérifie que chaque fichier de examples/ donne le résultat attendu :
// valid_*.json doit être valide, invalid_*.json doit être refusé.
// Les autres fichiers (ex : mon_esp.json) sont ignorés.

const fs = require('fs');
const path = require('path');
const { validateText } = require('./validator');

const dir = path.join(__dirname, 'examples');
let failures = 0;

for (const file of fs.readdirSync(dir).sort()) {
    if (!file.endsWith('.json') || !/^(valid|invalid)_/.test(file)) {
        continue;
    }

    const expected = file.startsWith('valid_');
    const result = validateText(fs.readFileSync(path.join(dir, file), 'utf8'));
    const ok = result.valid === expected;

    if (!ok) {
        failures++;
    }

    console.log(`${ok ? 'OK  ' : 'FAIL'} ${file} (attendu : ${expected ? 'valide' : 'non valide'})`);

    for (const message of result.errors) {
        console.log(`       - ${message}`);
    }
}

console.log(failures === 0 ? '\nTous les tests passent.' : `\n${failures} test(s) en échec.`);
process.exit(failures === 0 ? 0 : 1);
