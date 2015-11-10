#!/usr/bin/env python
# encoding: utf-8


# from datetime import datetime
import math


class BurstDetect(object):

    def __init__(self, time_interval, delta1, delta2, delta3):
        self.time_interval = time_interval
        self.delta1 = delta1
        self.delta2 = delta2
        self.delta3 = delta3
        self.date = set()
        self.content = dict()
        self.burst_word = dict()
        self.vec = dict()
        self.word2id = dict()
        self.id2word = dict()

    def get_f_r_w(self, date_file, content_file):
        f1 = open(date_file, 'r')
        f2 = open(content_file, 'r')
        for l1, l2 in zip(f1, f2):
            l1 = l1.split(' ')[0]
            self.date.add(l1)
            l2 = l2.split(' ')
            if self.content.has_key(l1):
                self.content[l1].append(l2)
            else:
                self.content[l1] = [l2]
        self.date = list(self.date)
        self.date.sort()

        pre_tf = dict()
        for day in self.date:
            tf = dict()
            df = dict()
            cont = self.content[day]
            for line in cont:
                for item in line:
                    tf[item] = tf[item] + 1 if tf.has_key(item) else 1
                for item in set(line):
                    df[item] = df[item] + 1 if df.has_key(item) else 1

            sum_tf = 0
            for k, v in tf.items():
                sum_tf += v**2
            abs_tf = dict()
            for k, v in tf.items():
                abs_tf[k] = v * 1.0 / sum_tf

            w = dict()
            leng = len(cont)
            for k, v in abs_tf.items():
                w[k] = abs_tf[k] * math.exp(df[k] / leng)

            r = dict()
            for k, v in tf.items():
                r[k] = v * 1.0 / pre_tf[k] if pre_tf.has_key(k) else 0


            tmp = sorted(tf.values(), reverse=True)
            self.delta1 = tmp[80]
            tmp = sorted(r.values(), reverse=True)
            self.delta2 = tmp[50]
            tmp = sorted(w.values(), reverse=True)
            self.delta3 = tmp[50]

            word = []
            for k, v in tf.items():
                if v > self.delta1 and r[k] > self.delta2 and w[k] > self.delta3:
                    word.append(k)
            self.burst_word[day] = word
            word_id = dict()
            id_word = dict()
            for i in xrange(len(word)):
                word_id[word[i]] = i
                id_word[i] = word[i]
            self.word2id[day] = word_id
            self.id2word[day] = id_word

            pre_tf = tf
            #print '*' * 30
            #for k, v in tf.items():
            #    print k, ' ', v, ' ', r[k], ' ', w[k]
        print self.burst_word['2015-06-20']


    def word2vec(self):
        for day in self.date:
            cont = self.content[day]
            vec_list = []
            for line in cont:
                tmp = [0 for item in self.burst_word]
                for item in line:
                    if self.word2id[day].has_key(item):
                        tmp[self.word2id[day][item]] = 1
                vec_list.append(tmp)
            self.vec[day] = vec_list

    def get_multify(self, vec1, vec2):
        sum = 0
        for i in xrange(len(vec1)):
            sum += vec1[i] * vec2[i]
        return sum

    def get_similar(self, input_vec):
        similar = []
        for vec1 in input_vec:
            tmp = []
            for vec2 in input_vec:
                td = min(len(vec1), len(vec2)) - self.get_multify(vec1, vec2)
                if len(vec1) > 3 and len(vec2) > 3:
                    if td <= 2.0:
                        tmp.append(1)
                    else:
                        tmp.append(0)
                else:
                    if td < 2.0:
                        tmp.append(1)
                    else:
                        tmp.append(0)
            similar.append(tmp)
        return similar

    def is_similar(self, vec1, vec2):
        len1 = sum(vec1)
        len2 = sum(vec2)
        td = min(len1, len2) - self.get_multify(vec1, vec2)
        if len1 > 3 and len2 > 3:
            if td <= 2.0:
                return True
            else:
                return False
        else:
            if td < 2.0:
                return True
            else:
                return False

    def cluster(self, input_vec, day):
        centroid = []
        cnt = 0
        while True:
            if sum(input_vec[cnt]) >= 3:
                centroid.append([cnt])
                cnt += 1
                break
            cnt += 1
            if cnt == len(input_vec):
                break
        for i in xrange(cnt, len(input_vec)):
            if sum(input_vec[i]) < 3:
                continue
            flag1 = True
            for j in xrange(len(centroid)):
                flag2 = True
                for item in centroid[j]:
                    if not self.is_similar(input_vec[i], input_vec[item]):
                        flag2 = False
                        break
                if flag2:
                    centroid[j].append(i)
                    flag1 = False
                    break
            if flag1:
                centroid.append([i])
        return None
        print
        print '-' * 100
        print day
        print 'the number of doc is ', len(input_vec)
        print 'centroid number is ', len(centroid)
        for line in centroid:
            print 'one centroid len is ', len(line)
            for item in line:
                print self.content[day][item]
            for item in line:
                print self.vec[day][item]

    def process(self):
        self.word2vec()
        for day in self.date:
            self.cluster(self.vec[day], day)


if __name__ == '__main__':
    test = BurstDetect('1 day', 3, 1.5, 0.001)
    test.get_f_r_w('date.txt', 'content.txt')
    test.process()



