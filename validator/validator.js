// Validation d'un statut ESP32 par rapport au schéma JSON du cours.
// Ce module ne fait aucune entrée/sortie : il pourra être réutilisé
// tel quel par le serveur du TP5.

const Ajv = require('ajv');
const schema = require('./schema.json');

const ajv = new Ajv({ allErrors: true, strict: true });
const validateSchema = ajv.compile(schema);

function formatPath(instancePath) {
    return instancePath === '' ? '(racine)' : instancePath.slice(1).replaceAll('/', '.');
}

function formatError(error) {
    const path = formatPath(error.instancePath);
    const p = error.params;

    switch (error.keyword) {
        case 'required':
            return `${path} : champ obligatoire manquant "${p.missingProperty}"`;
        case 'additionalProperties':
            return `${path} : champ non prévu par le modèle "${p.additionalProperty}"`;
        case 'type':
            return `${path} : type incorrect (attendu : ${p.type})`;
        case 'enum':
            return `${path} : valeur non autorisée (attendu : ${p.allowedValues.join(' ou ')})`;
        case 'minimum':
        case 'maximum':
            return `${path} : valeur hors limites (${p.comparison} ${p.limit})`;
        case 'minLength':
            return `${path} : texte vide`;
        case 'pattern':
            return `${path} : format incorrect`;
        case 'anyOf':
            return `${path} : valeur incorrecte (attendu : "NOP" ou une adresse IPv4)`;
        default:
            return `${path} : ${error.message}`;
    }
}

// Valide une chaîne de caractères contenant un JSON.
// Retourne { valid: boolean, errors: string[] }.
function validateText(text) {
    if (typeof text !== 'string' || text.trim() === '') {
        return { valid: false, errors: ['contenu vide'] };
    }

    let data;

    try {
        data = JSON.parse(text.replace(/^﻿/, ''));
    } catch (error) {
        return { valid: false, errors: [`syntaxe JSON invalide : ${error.message}`] };
    }

    return validateObject(data);
}

// Valide un objet JavaScript déjà désérialisé.
function validateObject(data) {
    if (validateSchema(data)) {
        return { valid: true, errors: [] };
    }

    // Pour un anyOf, on garde seulement le message global de ce champ
    const anyOfPaths = validateSchema.errors
        .filter((error) => error.keyword === 'anyOf')
        .map((error) => error.instancePath);

    const errors = validateSchema.errors
        .filter((error) => error.keyword === 'anyOf' || !anyOfPaths.includes(error.instancePath))
        .map(formatError);

    return { valid: false, errors };
}

module.exports = { validateText, validateObject };
