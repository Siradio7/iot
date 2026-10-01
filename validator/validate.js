// Usage : node validate.js <fichier.json> [autres fichiers...]
// Code de sortie : 0 si tous les fichiers sont valides, 1 sinon, 2 si mauvais usage.

const fs = require('fs');
const { validateText } = require('./validator');

function readFile(file) {
    try {
        return { text: fs.readFileSync(file, 'utf8') };
    } catch (error) {
        if (error.code === 'ENOENT') {
            return { error: 'fichier introuvable' };
        }

        if (error.code === 'EISDIR') {
            return { error: "c'est un dossier, pas un fichier" };
        }

        return { error: `lecture impossible (${error.code || error.message})` };
    }
}

const files = process.argv.slice(2);

if (files.length === 0) {
    console.error('Usage : node validate.js <fichier.json> [autres fichiers...]');
    process.exit(2);
}

let allValid = true;

for (const file of files) {
    const { text, error } = readFile(file);
    const result = error ? { valid: false, errors: [error] } : validateText(text);

    if (result.valid) {
        console.log(`✅ ${file} : VALIDE`);
        continue;
    }

    allValid = false;
    console.log(`❌ ${file} : NON VALIDE`);

    for (const message of result.errors) {
        console.log(`   - ${message}`);
    }
}

process.exit(allValid ? 0 : 1);
