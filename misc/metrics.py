#!/usr/bin/python3
# SPDX-License-Identifier: zlib-acknowledgement

# Specifically, this means that an entire conceptual changeset ("add a foo widget") is represented in the remote as exactly one commit (in some form), not a sequence of checkpoint commits.

# AWS High Level Services, e.g. Compute (EC2 for virtual machine)/Storage/Database etc.

# want these for things in master branch only?

# .text, .bss, .data, commit, build
#    revision: a2bc51465395057e3830614b20ceea10976204bb
#    parent_revision: c1fdf98ba5eb393a7646775921630bf4323ed832
#    build: lwm2m_client
#    text: 130,780 B
#    data: 3,356 B
#    bss: 21,167 B

import os
import sys

from dataclasses import dataclass

from urllib.parse import urlparse

import psycopg2

@dataclass
class CodeSizeTableEntry:
  created_at: str
  git_hash: str 
  parent_git_hash: str
  build_type: str
  text_bytes: int
  data_bytes: int
  bss_bytes: int

def fatal_error(msg):
  print(msg)
  breakpoint()
  sys.exit()

def main():
  # NOTE(Ryan): Disable breakpoints if not running under a debugger
  if sys.gettrace() is None:
    os.environ["PYTHONBREAKPOINT"] = "0"

  if "EMBEDDED_DB_URL" not in os.environ:
    fatal_error("Environment variable EMBEDDED_DB_URL not set")

  raw_db_url = os.environ.get("EMBEDDED_DB_URL", "postgresql://localhost:5432/embedded")
  try:
    parsed_db_url = urlparse(raw_db_url)
  except ValueError as e:
    fatal_error(e)

  db_name = parsed_db_url.path[1:]
  db_username = parsed_db_url.username
  db_password = parsed_db_url.password
  db_host = parsed_db_url.hostname
  db_port = parsed_db_url.port

  try:
    db_connection = psycopg2.connect(dbname=db_name, user=db_username, password=db_password, host=db_host, port=db_port)
  except psycopg2.Error as e:
    fatal_error(e)

  db_cursor = db_connection.cursor()
  db_cursor.execute("""
      CREATE TABLE "CodeSizes" (
        "created_at" timestamp NOT NULL DEFAULT NOW(),
        "hash" varchar NOT NULL,
        "parent_hash" varchar NOT NULL,
        "build_type" varchar NOT NULL,
        "text" int4 NOT NULL,
        "data" int4 NOT NULL,
        "bss" int4 NOT NULL,
        PRIMARY KEY ("hash","build_type")
      )""")
  db_connection.commit()


codesize = CodesizeData(
    # class with all code size info
)
cursor.execute("""
    INSERT INTO codesizes
        (revision, build, parent_revision, text, data, bss, message)
    VALUES ('{}', '{}', '{}', {}, {}, {}, '{}')
    """.format(codesize.revision, codesize.build,
               codesize.parent_revision,
               codesize.text, codesize.data,
               codesize.bss, codesize.message
    )
)
conn.commit()
   
cursor.execute("""
    SELECT
      revision, build, parent_revision,
      text, data, bss, message
    FROM codesizes
    WHERE build = '{}'
      AND revision = '{}'
    """.format(build, revision)
)
record = cursor.fetchone()
revision, build, parent_revision, text, data, bss, message = record
return CodesizeData(
    # Fill in class data
)

def _upload_codesize(codesize):
    with db_conn() as conn:
        cursor = conn.cursor()
        cursor.execute(
            """
            INSERT INTO codesizes (revision, build, parent_revision, text, data, bss, message)
            VALUES ('{REVISION}', '{BUILD}', '{PARENT_REVISION}', {TEXT}, {DATA}, {BSS}, '{MESSAGE}')
            """.format(
                REVISION=codesize.revision,
                BUILD=codesize.build,
                PARENT_REVISION=codesize.parent_revision,
                TEXT=codesize.text,
                DATA=codesize.data,
                BSS=codesize.bss,
                MESSAGE=codesize.message,
            )
        )
        conn.commit()


def _fetch_codesize(build, revision="HEAD"):
    with db_conn() as conn:
        cursor = conn.cursor()
        cursor.execute(
            """SELECT revision, build, parent_revision, 
                      text, data, bss, message 
               FROM codesizes 
               WHERE build = '{}' 
                 AND revision = '{}'""".format(
                build, revision
            )
        )
        record = cursor.fetchone()
        revision, build, parent_revision, text, data, bss, message = record
        return CodesizeData(
            revision=revision,
            build=build,
            parent_revision=parent_revision,
            text=text,
            data=data,
            bss=bss,
            message=message,
        )

  
def _calculate_codesize(binary_path):
    output = subprocess.check_output(
        ["arm-none-eabi-size", binary_path], encoding="UTF-8"
    )
    # Get the second line
    size_output = output.splitlines()[1]

    # Pull out text, data, and bss
    text, data, bss, *_ = size_output.split()
    return (int(text), int(data), int(bss))


def _convert_revision_to_git_sha(revision):
    return subprocess.check_output(
        ["git", "rev-parse", "{}".format(revision)], encoding="UTF-8"
    ).strip()

if __name__ == "__main__":
  main()
