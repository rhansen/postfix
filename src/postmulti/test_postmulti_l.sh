#!/bin/sh
set -e

mydir=$(cd "${0%/*}" && pwd) || exit 1
topdir=$(cd "${mydir}"/../.. && pwd) || exit 1
testtmp=${1:-${mydir}/test_postmulti_l_instances}

postmulti() { ${VALGRIND} ./postmulti -c "${testtmp}"/etc/postfix "$@"; }

rm -rf "${testtmp}"

# 'postmulti -e create' requires root.  Manually create multiple instances so
# that the '-l' tests don't need to run as root.
for inst in "" -secondary; do
    for dir in data etc queue; do
	mkdir -p "${testtmp}/${dir}/postfix${inst}"
    done
    # multi_instance_* will be set later.
    cat <<EOF >${testtmp}/etc/postfix${inst}/main.cf
command_directory = ${topdir}/bin
daemon_directory = ${topdir}/libexec
data_directory = ${testtmp}/data/postfix${inst}
queue_directory = ${testtmp}/queue/postfix${inst}
maillog_file = /dev/stdout
EOF
    cat <<EOF >${testtmp}/etc/postfix${inst}/master.cf
postlog unix-dgram n - n - 1 postlogd
EOF
    touch "${testtmp}/etc/postfix${inst}/master.cf"
done

primary_cf=${testtmp}/etc/postfix/main.cf
secondary_cf=${testtmp}/etc/postfix-secondary/main.cf

echo ">>> 1: single instance (multi_instance_enable = no)"
postmulti -l
postmulti -l -j

echo ">>> 2: single instance (multi_instance_enable = yes)"
cat <<EOF >>${primary_cf}
multi_instance_wrapper = ${command_directory}/postmulti -p --
multi_instance_enable = yes
EOF
postmulti -l
postmulti -l -j

echo ">>> 3: multiple instances, secondary name=false group=false"
cat <<EOF >>${primary_cf}
multi_instance_directories = ${testtmp}/etc/postfix-secondary
EOF
postmulti -l
postmulti -l -j

echo ">>> 4: multiple instances, secondary name=true group=false"
cat <<EOF >>${secondary_cf}
multi_instance_name = postfix-secondary
EOF
postmulti -l
postmulti -l -j

echo ">>> 5: multiple instances, secondary name=true group=true"
cat <<EOF >>${secondary_cf}
multi_instance_group = secondary
EOF
postmulti -l
postmulti -l -j

echo ">>> 6: multiple instances, secondary name=false group=true"
sed -e '/multi_instance_name/d' "${secondary_cf}" >${secondary_cf}.new
mv "${secondary_cf}".new "${secondary_cf}"
postmulti -l
postmulti -l -j
