#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import sys
import shutil
import locale
import codecs


def write_file(path, content):
    with codecs.open(path, 'w', 'utf-8') as f:
        f.write(content)


def check_file(path, content):
    file_content = ''
    try:
        with codecs.open(path, 'r', 'utf-8') as f:
            file_content = f.read()
    except:
        file_content = None
    assert file_content == content, 'Check file content failed, expected: "%s", actual: "%s"' % (
        content if content is not None else '<None>', file_content if file_content is not None else '<None>')


def test_single_file(zip_cmd, unzip_cmd):
    write_file('test_root/f1', 'content')
    os.system('%s test_root/test.zip test_root/f1' % zip_cmd)
    os.system('%s test_root/test.zip test_root/unzip' % unzip_cmd)
    check_file('test_root/unzip/f1', 'content')


def test_single_directory(zip_cmd, unzip_cmd):
    os.makedirs('test_root/d1')
    write_file('test_root/d1/f1', 'content')
    os.system('%s test_root/test.zip test_root/d1' % zip_cmd)
    os.system('%s test_root/test.zip test_root/unzip' % unzip_cmd)
    check_file('test_root/unzip/d1/f1', 'content')


def test_directory_tree(zip_cmd, unzip_cmd):
    os.makedirs('test_root/d1/d2/d3')
    write_file('test_root/d1/f1', 'content1')
    write_file('test_root/d1/d2/f2', 'content2')
    write_file('test_root/d1/d2/d3/f3', 'content3')
    os.system('%s test_root/test.zip test_root/d1' % zip_cmd)
    os.system('%s test_root/test.zip test_root/unzip' % unzip_cmd)
    check_file('test_root/unzip/d1/f1', 'content1')
    check_file('test_root/unzip/d1/d2/f2', 'content2')
    check_file('test_root/unzip/d1/d2/d3/f3', 'content3')


def test_wildcard1(zip_cmd, unzip_cmd):
    os.makedirs('test_root/d1')
    write_file('test_root/d1/f1', 'content1')
    os.system('%s test_root/test.zip "test_root/d1/*"' % zip_cmd)
    os.system('%s test_root/test.zip test_root/unzip' % unzip_cmd)
    check_file('test_root/unzip/f1', 'content1')


def test_wildcard2(zip_cmd, unzip_cmd):
    os.makedirs('test_root/d1/d2/d22')
    os.makedirs('test_root/d1/d2/d33')
    write_file('test_root/d1/f1', 'content1')
    write_file('test_root/d1/d2/f2', 'content2')
    write_file('test_root/d1/d2/d22/f22', 'content22')
    write_file('test_root/d1/d2/d33/f33', 'content33')
    os.system('%s test_root/test.zip "test_root/d1/*2"' % zip_cmd)
    os.system('%s test_root/test.zip test_root/unzip' % unzip_cmd)
    check_file('test_root/unzip/f1', None)
    check_file('test_root/unzip/d2/f2', 'content2')
    check_file('test_root/unzip/d2/d22/f22', 'content22')
    check_file('test_root/unzip/d2/d33/f33', 'content33')


def test_non_ascii_file_name(zip_cmd, unzip_cmd):
    os.makedirs('test_root/目录1/目录2/目录3')
    write_file('test_root/目录1/文件1', u'内容1')
    write_file('test_root/目录1/目录2/文件2', u'内容2')
    write_file('test_root/目录1/目录2/目录3/文件3', u'内容3')
    os.system('%s test_root/测试.zip test_root/目录1' % zip_cmd)
    os.system('%s test_root/测试.zip test_root/解压' % unzip_cmd)
    check_file('test_root/解压/目录1/文件1', u'内容1')
    check_file('test_root/解压/目录1/目录2/文件2', u'内容2')
    check_file('test_root/解压/目录1/目录2/目录3/文件3', u'内容3')


def run_tests():
    if sys.platform == 'win32':
        zip_cmd = 'zip.exe'
        unzip_cmd = 'unzip.exe'
        zipa_cmd = 'zipa.exe'
        unzipa_cmd = 'unzipa.exe'
    else:
        zip_cmd = './zip'
        unzip_cmd = './unzip'

    for test_function in (
        test_single_file,
        test_single_directory,
        test_directory_tree,
        test_wildcard1,
        test_wildcard2,
        test_non_ascii_file_name,
    ):
        if os.path.exists('test_root'):
            shutil.rmtree('test_root')
        os.makedirs('test_root')
        test_function(zip_cmd, unzip_cmd)

    if sys.platform == 'win32':
        for test_function in (
            test_single_file,
            test_single_directory,
            test_directory_tree,
            test_wildcard1,
            test_wildcard2,
            test_non_ascii_file_name,
        ):
            if os.path.exists('test_root'):
                shutil.rmtree('test_root')
            os.makedirs('test_root')
            if test_function != test_non_ascii_file_name or locale.getpreferredencoding() in ('cp65001', 'cp936'):
                test_function(zipa_cmd, unzipa_cmd)
    # shutil.rmtree('test_root')


def main():
    (out_dir,) = sys.argv[1:]
    os.chdir(out_dir)
    run_tests()


if __name__ == '__main__':
    main()
