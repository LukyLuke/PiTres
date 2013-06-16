#!/bin/sh

UID=${1-921}
ST=${2-st=ts}
LDIF=${3}

if [ -z "${LDIF}" ];
then
  echo "Usage: $0 UID ST LDIF"
  echo "  UID:  Your member uid in ldap: 921"
  echo "  ST:   Your section string: st=ts"
  echo "  LDIF: The LDIF which holds the update"
  echo;
  echo "  With the UID and the ST, the bind-dn is built like:"
  echo "    uniqueIdentifier=UID,dc=members,ST,dc=piratenpartei,dc=ch"
  echo "  Therefore the ST must be a dn part is you're in a subsection:"
  echo "    l=bern,st=be"
  echo;
  exit 1;
fi

ldapmodify -D "uniqueIdentifier=${UID},dc=members,${ST},dc=piratenpartei,dc=ch" -h localhost -p 1389 -W -c -f ${LDIF} -v > ${LDIF}.log
