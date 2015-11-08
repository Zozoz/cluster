#!/usr/bin/env python
# encoding: utf-8


import math
import sys
from lxml import etree

import numpy as np


def parse_xml():
    tree = etree.parse('nysk.xml')
    docid = tree.xpath('//docid/text()')
    source = tree.xpath('//source/text()')
    url = tree.xpath('//url/text()')
    title = tree.xpath('//title/text()')
    summary = tree.xpath('//summary/text()')
    text = tree.xpath('//text/text()')
    date = tree.xpath('//date/text()')

    fp = open('summary.data', 'w')
    for item in title:
        fp.write(item.encode('utf-8') + '\n')
    fp.close()

    return summary

def get_tf_idf(word_list):
    X = []
    XX = []
    for item in word_list:
        X.append([it.strip('.').strip(')').strip('(').strip('\n') for itt in item.split(' ') for it in itt.split('.')])
        XX.append(list({it.strip('.').strip(')').strip('(').strip('\n') for itt in item.split(' ') for it in itt.split('.')}))

    V = list({item for line in X for item in line})
    length = len(V)
    word_id = {}
    for i in xrange(length):
        word_id[V[i]] = i

    vec = []
    cnt = 0
    for line in X:
        count_term = dict()
        for item in line:
            if item:
                count_term[word_id[item]] = count_term[word_id[item]] + 1 if count_term.has_key(word_id[item]) else 1
        vec.append(count_term)
        cnt += 1
        if cnt > 1000:
            break

    count_doc = dict()
    cnt = 0
    for line in XX:
        for item in line:
            if item:
                count_doc[word_id[item]] = count_doc[word_id[item]] + 1 if count_doc.has_key(word_id[item]) else 1
        cnt += 1
        if cnt > 1000:
            break

    tf_idf = []
    f = set()
    for line, x in zip(vec, X):
        count_tf_idf = dict()
        for k, v in line.items():
            tmp = v * 1.0  / len(x) * math.log(cnt / count_doc[k])
            if tmp > 0.1:
                count_tf_idf[k] = v
                f.add(k)
        tf_idf.append(count_tf_idf)

    return tf_idf

    f = list(f)
    print 'length of f is ', len(f)
    dict_f = dict()
    for i in xrange(len(f)):
        dict_f[f[i]] = i

    ans = []
    for line in tf_idf:
        feat = np.zeros((len(f), 1))
        for k, v in line.items():
            if k in f:
                feat[dict_f[k]] = 1
        ans.append(feat)
    return ans


def process_data(word_list):
    X = []
    for item in word_list:
        X.append([it.strip('.').strip(')').strip('(').strip('\n') for itt in item.split(' ') for it in itt.split('.')])

    V = list({item for line in X for item in line})
    length = len(V)
    word_id = {}
    for i in xrange(length):
        word_id[V[i]] = i
    vec = []
    cnt = 0
    for line in X:
        count_term = dict()
        for item in line:
            if item:
                count_term[word_id[item]] = count_term[word_id[item]] + 1 if count_term.has_key(word_id[item]) else 1
        vec.append(count_term)
        cnt += 1
        if cnt > 1000:
            break

    return vec

    #count_term = sorted(count_term.iteritems(), key=lambda d:d[1], reverse=True)
    #for item in count_term:
    #    print item
    #count_term = dict(count_term)

def word2vec(word_list):
    vec_list = []
    X = []
    cnt = 0
    for item in word_list:
        cnt += 1
        if cnt > 1000:
            break
        X.append(list({it.strip('.').strip(')').strip('(').strip('\n') for itt in item.split(' ') for it in itt.split('.')}))
    V = list({item for line in X for item in line})
    length = len(V)

    word_id = {}
    for i in xrange(length):
        word_id[V[i]] = i
    for line in X:
        item = np.zeros((length, 1))
        for it in line:
            item[word_id[it]] = 1
        vec_list.append(item)

    return vec_list

def _get_distance(vec1, vec2):
    dist = 0.0
    for k, v in vec1.items():
        if vec2.has_key(k):
            dist += (vec2[k] - v) ** 2
        else:
            dist += v ** 2
    for k, v in vec2.items():
        if not vec1.has_key(k):
            dist += v ** 2
    return math.log(dist + 1.)
    # return sum(abs(vec1 - vec2))

def get_distance(vec_list):
    length = len(vec_list)
    print 'vec_list is ', length
    dist = np.zeros((length, length))
    for i in xrange(length):
        print i
        for j in xrange(length):
            if i == j:
                dist[i, j] = sys.maxint
            else:
                dist[i, j] = _get_distance(vec_list[i], vec_list[j])
    return dist

def cluster(vec_list, max_iter):
    length = len(vec_list)
    # length = 6
    centroid = dict()
    for i in xrange(length):
        centroid[str(i)] = [i]
    dist = np.mat(get_distance(vec_list))
    # dist = np.mat([[1000, 662, 877, 255, 412, 996],
    #                [662, 1000, 295, 468, 268, 400],
    #                [877, 295, 1000, 754, 564, 138],
    #                [255, 468, 764, 1000, 219, 869],
    #                [412, 268, 564, 219, 1000, 669],
    #                [996, 400, 138,869, 669, 1000]])
    cnt = 0
    while True:
        cnt += 1
        print '-' * 10, cnt, '-' * 10
        print dist
        min_index = dist.argmin()
        row = min_index / length
        col = min_index % length
        print 'row=', row, 'col=', col
        for i in xrange(length):
            if i != row and dist[row, i] > dist[col, i]:
                dist[row, i] = dist[col, i]
            dist[col, i] = sys.maxint
            if i != row and dist[i, row] > dist[i, col]:
                dist[i, row] = dist[i, col]
            dist[i, col] = sys.maxint
        centroid[str(row)].extend(centroid[str(col)])
        del centroid[str(col)]

        for k in centroid.items():
            print k

        if cnt > max_iter:
            break
    return centroid


if __name__ == '__main__':
    # vec_list = process_data(parse_xml())
    vec_list = get_tf_idf(parse_xml())
    fp = open('single.txt', 'w')
    #for item in vec_list:
    #    for i in xrange(len(item)):
    #        fp.write(str(i) + ':' + str(item[i, 0]) + ' ')
    #    fp.write('\n')
    for item in vec_list:
        for k, v in item.items():
            fp.write(str(k) + ':' + str(v) + ' ')
        if not item:
            fp.write('1:0.001 ')
        fp.write('\n')
    # vec_list = word2vec(parse_xml())
    # print sum(vec_list[1])
    # cluster(vec_list, 500)


