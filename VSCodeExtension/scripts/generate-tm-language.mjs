import { readFile, writeFile } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';

const grammarPath = fileURLToPath(new URL('../syntaxes/glsl.tmLanguage.json', import.meta.url));
const lexicalRoot = new URL('../../glsld/Database/Lexicals/', import.meta.url);

async function readWords(relativePaths) {
	const words = new Set();
	for (const relativePath of relativePaths) {
		const source = await readFile(new URL(relativePath, lexicalRoot), 'utf8');
		for (const word of source.split(/\s+/u)) {
			if (/^[A-Za-z_][A-Za-z0-9_]*$/u.test(word)) { words.add(word); }
		}
	}
	return [...words].sort((left, right) => left < right ? -1 : left > right ? 1 : 0);
}

function escapeRegex(word) {
	return word.replace(/[.*+?^${}()|[\]\\]/gu, '\\$&');
}

function buildRule(name, words, sources) {
	if (words.length === 0) { throw new Error(`No lexical entries loaded from ${sources.join(', ')}`); }
	const chunkSize = 192;
	const patterns = [];
	for (let offset = 0; offset < words.length; offset += chunkSize) {
		patterns.push({ name, match: `\\b(?:${words.slice(offset, offset + chunkSize).map(escapeRegex).join('|')})\\b` });
	}
	return { comment: `Generated from ${sources.join(', ')}. Do not edit this rule manually.`, patterns };
}

function buildFunctionDefinitionRule() {
	const identifier = '[A-Za-z_][A-Za-z0-9_]*';
	const nextFunction = `^\\s*(?:(?:${identifier})\\s+)+(?:${identifier})\\s*\\(`;
	return {
		comment: 'Generated as a root-only range. Function bodies use statement_context and cannot recursively match another function definition.',
		name: 'meta.function.definition.glsl',
		begin: `^(\\s*)((?:${identifier}\\s+)*)(${identifier})(\\s+)(${identifier})(?=\\s*\\()`,
		beginCaptures: {
			2: {
				patterns: [
					{ include: '#metadata_qualifiers' }
				]
			},
			3: {
				patterns: [
					{ include: '#metadata_types' },
					{ name: 'entity.name.type.return.glsl', match: identifier }
				]
			},
			5: { name: 'entity.name.function.definition.glsl' }
		},
		end: `(?<=\\})|(?<=;)|(?=${nextFunction})`,
		patterns: [
			{ include: '#function_parameters' },
			{ include: '#comments' },
			{ include: '#preprocessors' },
			{ include: '#attributes' },
			{ include: '#compound_statement' },
			{ include: '#punctuation' }
		]
	};
}

function buildParameterDeclarationsRule() {
	const identifier = '[A-Za-z_][A-Za-z0-9_]*';
	const parameterEnd = '(?=\\s*(?:\\[|,|\\)))';
	return {
		patterns: [
			{
				name: 'meta.parameter.glsl',
				match: `(?:(?<=\\()|(?<=,))\\s*((?:${identifier}\\s+)+)(${identifier})${parameterEnd}`,
				captures: {
					1: {
						patterns: [
							{ include: '#metadata_qualifiers' },
							{ include: '#metadata_types' },
							{ name: 'entity.name.type.parameter.glsl', match: identifier }
						]
					},
					2: { name: 'variable.parameter.glsl' }
				}
			},
			{
				name: 'meta.parameter.unnamed.glsl',
				match: `(?:(?<=\\()|(?<=,))\\s*(${identifier})${parameterEnd}`,
				captures: {
					1: {
						patterns: [
							{ include: '#metadata_types' },
							{ name: 'entity.name.type.parameter.glsl', match: identifier }
						]
					}
				}
			}
		]
	};
}

function installContexts(grammar) {
	grammar.patterns = [{ include: '#root_context' }];
	grammar.repository.root_context = {
		patterns: [
			{ include: '#comments' },
			{ include: '#preprocessors' },
			{ include: '#strings_and_numbers' },
			{ include: '#attributes' },
			{ include: '#layout_qualifiers' },
			{ include: '#metadata_function_definitions' },
			{ include: '#structures' },
			{ include: '#compound_statement' },
			{ include: '#statement_tokens' }
		]
	};
	grammar.repository.statement_context = {
		patterns: [
			{ include: '#comments' },
			{ include: '#preprocessors' },
			{ include: '#strings_and_numbers' },
			{ include: '#attributes' },
			{ include: '#layout_qualifiers' },
			{ include: '#structures' },
			{ include: '#compound_statement' },
			{ include: '#statement_tokens' }
		]
	};
	grammar.repository.statement_tokens = {
		patterns: [
			{ include: '#metadata_keywords' },
			{ include: '#metadata_qualifiers' },
			{ include: '#declarations' },
			{ include: '#metadata_types' },
			{ include: '#constants' },
			{ include: '#member_functions' },
			{ include: '#members' },
			{ include: '#user_functions' },
			{ include: '#parentheses' },
			{ include: '#brackets' },
			{ include: '#identifiers' },
			{ include: '#operators' },
			{ include: '#punctuation' }
		]
	};
	grammar.repository.expression_context = {
		patterns: [
			{ include: '#comments' },
			{ include: '#preprocessors' },
			{ include: '#strings_and_numbers' },
			{ include: '#metadata_keywords' },
			{ include: '#metadata_qualifiers' },
			{ include: '#metadata_types' },
			{ include: '#constants' },
			{ include: '#member_functions' },
			{ include: '#members' },
			{ include: '#user_functions' },
			{ include: '#parentheses' },
			{ include: '#brackets' },
			{ include: '#identifiers' },
			{ include: '#operators' },
			{ include: '#punctuation' }
		]
	};
	grammar.repository.compound_statement = {
		name: 'meta.block.glsl',
		begin: '\\{',
		beginCaptures: { 0: { name: 'punctuation.section.braces.begin.glsl' } },
		end: '\\}',
		endCaptures: { 0: { name: 'punctuation.section.braces.end.glsl' } },
		patterns: [{ include: '#statement_context' }]
	};
	grammar.repository.parentheses = {
		name: 'meta.group.glsl',
		begin: '\\(',
		beginCaptures: { 0: { name: 'punctuation.section.parens.begin.glsl' } },
		end: '\\)',
		endCaptures: { 0: { name: 'punctuation.section.parens.end.glsl' } },
		patterns: [{ include: '#statement_context' }]
	};
	grammar.repository.brackets = {
		name: 'meta.brackets.glsl',
		begin: '\\[',
		beginCaptures: { 0: { name: 'punctuation.section.brackets.begin.glsl' } },
		end: '\\]',
		endCaptures: { 0: { name: 'punctuation.section.brackets.end.glsl' } },
		patterns: [{ include: '#expression_context' }]
	};
	grammar.repository.function_parameters = {
		name: 'meta.function.definition.parameters.glsl',
		begin: '\\G\\s*(\\()',
		beginCaptures: { 1: { name: 'punctuation.section.parameters.begin.glsl' } },
		end: '\\)',
		endCaptures: { 0: { name: 'punctuation.section.parameters.end.glsl' } },
		patterns: [
			{ include: '#parameter_declarations' },
			{ include: '#comments' },
			{ include: '#preprocessors' },
			{ include: '#strings_and_numbers' },
			{ include: '#metadata_qualifiers' },
			{ include: '#metadata_types' },
			{ include: '#brackets' },
			{ include: '#operators' },
			{ include: '#punctuation' }
		]
	};
	grammar.repository.parameter_declarations = buildParameterDeclarationsRule();
	grammar.repository.member_functions = {
		name: 'meta.function-call.member.glsl',
		begin: '(?<=\\.)(\\s*)([A-Za-z_][A-Za-z0-9_]*)(\\s*)(\\()',
		beginCaptures: {
			2: { name: 'entity.name.function.member.glsl' },
			4: { name: 'punctuation.section.arguments.begin.glsl' }
		},
		end: '\\)',
		endCaptures: { 0: { name: 'punctuation.section.arguments.end.glsl' } },
		patterns: [{ include: '#expression_context' }]
	};
	grammar.repository.user_functions = {
		name: 'meta.function-call.glsl',
		begin: '\\b([A-Za-z_][A-Za-z0-9_]*)(\\s*)(\\()',
		beginCaptures: {
			1: { name: 'entity.name.function.call.glsl' },
			3: { name: 'punctuation.section.arguments.begin.glsl' }
		},
		end: '\\)',
		endCaptures: { 0: { name: 'punctuation.section.arguments.end.glsl' } },
		patterns: [{ include: '#expression_context' }]
	};
	grammar.repository.members = {
		patterns: [
			{
				match: '(?<=\\.)(\\s*)([xyzwrgbastpq]{1,4})\\b',
				captures: { 2: { name: 'variable.other.member.swizzle.glsl' } }
			},
			{
				match: '(?<=\\.)(\\s*)([A-Za-z_][A-Za-z0-9_]*)\\b',
				captures: { 2: { name: 'variable.other.member.glsl' } }
			}
		]
	};
}

const builtinTypeSources = ['Builtins/Types.txt'];
const primitiveTypeSources = ['Primitives/Types.txt'];
const keywordSources = ['Keywords/Meta.txt'];
const qualifierSources = ['Primitives/Qualifiers.txt'];
const grammarSource = await readFile(grammarPath, 'utf8');
const grammar = JSON.parse(grammarSource);

const builtinTypes = await readWords(builtinTypeSources);
const primitiveTypes = await readWords(primitiveTypeSources);
const keywords = await readWords(keywordSources);
const qualifiers = await readWords(qualifierSources);

for (const legacyRule of ['builtin_functions', 'builtin_variables', 'extended_types', 'function_definitions', 'generic_builtins', 'keywords', 'types']) {
	delete grammar.repository[legacyRule];
}

grammar.repository.metadata_types = {
	comment: 'Generated from primitive and built-in Metadata type lists. Do not edit this rule manually.',
	patterns: [
		...buildRule('storage.type.glsl', primitiveTypes, primitiveTypeSources).patterns,
		...buildRule('entity.name.type.glsl', builtinTypes, builtinTypeSources).patterns
	]
};
grammar.repository.metadata_keywords = buildRule('keyword.control.glsl', keywords, keywordSources);
grammar.repository.metadata_qualifiers = buildRule('storage.modifier.glsl', qualifiers, qualifierSources);
grammar.repository.metadata_function_definitions = buildFunctionDefinitionRule();
installContexts(grammar);

const generated = `${JSON.stringify(grammar, null, '\t')}\n`;
if (generated !== grammarSource) { await writeFile(grammarPath, generated, 'utf8'); }
