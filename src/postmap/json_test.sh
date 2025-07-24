#!/bin/sh
set -e

pecho() { printf %s\\n "$*"; }
log() { pecho "$@" >&2; }
error() { log "ERROR: $@"; }
fatal() { error "$@"; exit 1; }

postmap() {
    ${VALGRIND} ./postmap -c . "$@"
}

mydir=${0%/*}
cd "${mydir}"

invalid_utf8=$(printf '\200\n')
sparkles_emoji_utf8=$(printf '\342\234\250\n')

echo ">>>>>> -q <not-hyphen> -j"
echo ">>> value -> \"value\""
postmap -q key -j static:value
echo ">>> UTF-8 encoded sparkles emoji value -> \"${sparkles_emoji_utf8}\""
postmap -q key -j "static:${sparkles_emoji_utf8}"
echo ">>> invalid UTF-8 value -> error"
postmap -q key -j "static:${invalid_utf8}" \
    && fatal "got 0 exit status, want non-zero"
echo ">>> invalid UTF-8 key but valid value -> \"value\""
postmap -q "${invalid_utf8}" -j static:value

echo ""
echo ">>>>>> -q - -j"
echo ">>> ASCII and valid UTF-8 keys, ASCII value -> objects"
postmap -q - -j static:value <<EOF
key
${sparkles_emoji_utf8}
EOF
echo ">>> ASCII and valid UTF-8 keys, valid UTF-8 value -> objects"
postmap -q - -j "static:${sparkles_emoji_utf8}" <<EOF
key
${sparkles_emoji_utf8}
EOF
echo ">>> ASCII and valid UTF-8 keys, invalid UTF-8 value -> error"
postmap -q - -j "static:${invalid_utf8}" <<EOF \
    && fatal "got 0 exit status, want non-zero"
key
${sparkles_emoji_utf8}
EOF
echo ">>> ASCII, invalid UTF-8, and valid UTF-8 keys, ASCII value -> error"
# Note: postmap_queries() flushes stdout after processing all keys, not after
# each successful lookup, and msg_fatal() doesn't flush before exiting, so the
# successful lookup of key "key" will not be printed.
postmap -q - -j static:value <<EOF \
    && fatal "got 0 exit status, want non-zero"
key
${invalid_utf8}
${sparkles_emoji_utf8}
EOF

for args in "-h" "-b" "-m -h"; do
    echo ""
    echo ">>>>>> -q - ${args} -j"
    echo ">>> valid keys and value -> ok"
    postmap -q - ${args} -j "static:${sparkles_emoji_utf8}" <<EOF
FieldName1: FieldBody1
FieldName2: FieldBody2
 with ${sparkles_emoji_utf8}

Body
${sparkles_emoji_utf8}
EOF
    echo ">>> invalid keys, valid value -> error"
    postmap -q - ${args} -j "static:${sparkles_emoji_utf8}" <<EOF \
	&& fatal "got 0 exit status, want non-zero"
FieldName1: FieldBody1
FieldName2: FieldBody2
 with ${invalid_utf8}

Body
${invalid_utf8}
EOF
    echo ">>> valid keys, invalid value -> error"
    postmap -q - ${args} -j "static:${invalid_utf8}" <<EOF \
	&& fatal "got 0 exit status, want non-zero"
FieldName1: FieldBody1
FieldName2: FieldBody2
 with ${sparkles_emoji_utf8}

Body
${sparkles_emoji_utf8}
EOF
done

echo ""
echo ">>>>>> -s -j"
echo ">>> valid keys and value -> ok"
postmap -s -j "inline:{key=${sparkles_emoji_utf8},${sparkles_emoji_utf8}=value}"
echo ">>> invalid key, valid value -> error"
postmap -s -j "inline:{key=${sparkles_emoji_utf8},${invalid_utf8}=value}" \
    && fatal "got 0 exit status, want non-zero"
echo ">>> valid key, invalid value -> error"
postmap -s -j "inline:{key=${invalid_utf8},${sparkles_emoji_utf8}=value}" \
    && fatal "got 0 exit status, want non-zero"

true
