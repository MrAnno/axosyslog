#!/usr/bin/env python3
#############################################################################
# Copyright (c) 2024 Axoflow
# Copyright (c) 2024 Attila Szakacs <attila.szakacs@axoflow.com>
# Copyright (c) 2020 Balabit
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
# As an additional exemption you are allowed to compile & link against the
# OpenSSL libraries as published by the OpenSSL project. See the file
# COPYING for details.
#
#############################################################################

import re
import sys
import textwrap
from argparse import ArgumentParser
from pathlib import Path
from subprocess import PIPE, Popen

news_dir = Path(__file__).resolve().parent
root_dir = news_dir.parent
newsfile = root_dir / 'NEWS.md'

exclude_contributor_list = [
    "github-actions",
    "dependabot[bot]",
]


blocks = [
    ('Features', 'feature-*.md'),
    ('Bugfixes', 'bugfix-*.md'),
    ('FilterX features', 'fx-feature-*.md'),
    ('FilterX bugfixes', 'fx-bugfix-*.md'),
    ('Packaging', 'packaging-*.md'),
    ('Notes to developers', 'developer-note-*.md'),
    ('Other changes', 'other-*.md'),
]


def print_usage_if_needed():
    ArgumentParser(usage="\rCreates NEWS.md file from the entries in the news/ folder.\n"
                         "It also deletes the entry files.").parse_args()


def _exec(command):
    proc = Popen(command, cwd=str(root_dir), stdout=PIPE, shell=True)
    stdout, _ = proc.communicate()
    stdout = stdout.decode()
    return stdout


def create_block(block_name, files):
    block = '## {}\n\n'.format(block_name)
    for f in files:
        entry = ''
        match_pr_id = re.search(r'(\d+)(-\d+)?.md$', f.name)
        if not match_pr_id:
            sys.exit('Invalid filename: {}'.format(f.name))

        pr_id = match_pr_id.group(1)

        entry += '  * {}\n([#{}](https://github.com/axoflow/axosyslog/pull/{}))'.format(f.read_text().rstrip(), pr_id, pr_id)
        entry = entry.replace('\n', '\n    ')
        entry = entry.replace('\n    \n', '\n\n')
        block += entry + '\n\n'
    block += '\n'
    return block


def get_last_version():
    stdout = _exec(r'git show HEAD:NEWS.md')
    return stdout[:stdout.index('\n')]


def get_next_version():
    next_version = (root_dir / 'VERSION.txt').read_text().rstrip()
    return next_version


def create_version():
    next_version = get_next_version()
    return '{}\n{}\n\n'.format(next_version, len(next_version) * '=')


def create_prelude():
    return 'AxoSyslog is binary-compatible with syslog-ng [1] and serves as a drop-in replacement.\n\n' \
           'We provide [cloud-ready container images](https://github.com/axoflow/axosyslog/#container-images) and Helm charts.\n\n' \
           'Packages are available in our [APT](https://github.com/axoflow/axosyslog/#deb-packages) and [RPM](https://github.com/axoflow/axosyslog/#rpm-packages) ' \
           'repositories (Ubuntu, Debian, AlmaLinux, Fedora).\n\n' \
           'Check out the [AxoSyslog documentation](https://axoflow.com/docs/axosyslog-core/) for all the details.\n\n'

def create_highlights_block():
    return '## Highlights\n' \
           '\n' \
           '<Fill this block manually from the blocks below>\n' \
           '\n'


def create_standard_blocks():
    standard_blocks = ''
    for block_name, glob in blocks:
        entries = list(news_dir.glob(glob))
        if len(entries) > 0:
            standard_blocks += create_block(block_name, entries)
    return standard_blocks


def check_if_news_entries_are_present():
    return any(news_dir.glob("*-*.md"))


def create_discord_block():
    return '\n[1] syslog-ng is a trademark of One Identity.\n\n' \
           '## Discord\n' \
           '\n' \
           'For a bit more interactive discussion, join our Discord server:\n' \
           '\n' \
           '[![Axoflow Discord Server](https://discordapp.com/api/guilds/1082023686028148877/widget.png?style=banner2)](https://discord.gg/E65kP9aZGm)\n' \
           '\n'


def create_credits_block():
    def wrap(contributors):
        concated = ", ".join([c.replace(" ", "\0") for c in contributors])
        return textwrap.fill(concated, width=70).replace("\0", " ")

    stdout = _exec(r'git rev-list --no-merges --format=format:%aN axosyslog-' + get_last_version() + r'..HEAD | '
                   r'grep -Ev "^commit [a-z0-9]{40}$" | sort | uniq')
    contributors = stdout.rstrip().split('\n')
    contributors = filter(lambda x : x not in exclude_contributor_list, contributors)
    contributors = sorted(set(contributors))

    return '## Credits\n' \
           '\n' \
           'AxoSyslog is developed as a community project, and as such it relies\n' \
           'on volunteers, to do the work necessary to produce AxoSyslog.\n' \
           '\n' \
           'Reporting bugs, testing changes, writing code or simply providing\n' \
           'feedback is an important contribution, so please if you are a user\n' \
           'of AxoSyslog, contribute.\n' \
           '\n' \
           'We would like to thank the following people for their contribution:\n' \
           '\n' \
           '{}\n'.format(wrap(contributors))


def create_newsfile(news):
    newsfile.write_text(news)
    print('Newsfile created at {}\n'.format(newsfile.resolve()))


def cleanup():
    print("Cleaning up entry files with `git rm news/*-*.md`:")
    _exec("git rm news/*-*.md")


def create_news_content():
    news = create_version()
    news += create_prelude()
    news += create_highlights_block()
    news += create_standard_blocks()
    news += create_discord_block()
    news += create_credits_block()
    return news

def check_if_news_is_already_uptodate():
    return get_last_version() == get_next_version()


def main():
    print_usage_if_needed()

    if check_if_news_is_already_uptodate():
        if check_if_news_entries_are_present():
            print('NEWS.md file is already up-to-date with VERSION.txt but news entries still exist (news/*.md).\n'
                  'Remove NEWS entries or bump the VERSION.txt file.\n')
            return 1
        print("NEWS file is already up-to-date, no new NEWS entries, assuming it has been manually prepared")
    else:
        news = create_news_content()
        create_newsfile(news)
        cleanup()
    return 0


if __name__ == '__main__':
    sys.exit(main())
