#!/usr/bin/env python
# encoding: utf-8


import numpy as np

class SinglePass(object):

    def __init__(self, threshold):
        self.threshold = threshold
        self.word_vec = [] # data vector
        self.centroid = [] # index of every cluster
        self.cluster = [] # cluster center point

    def load_training_data(self, filename):
        with open(filename, 'r') as fp:
            for line in fp:
                self.word_vec.append([float(item) for item in line.split(' ')])

        self.word_vec = np.array(self.word_vec)

    def learn(self):
        self.centroid.append([0])
        self.cluster.append(self.word_vec[0])
        for i in xrange(1, len(self.word_vec)):
            MAXC = -1
            index = -1
            for j in xrange(len(self.centroid)):
                dist = self.word_vec[i].dot(self.cluster[j])
                if MAXC < dist:
                    MAXC = dist
                    index = j
            if MAXC < self.threshold:
                self.centroid.append([i])
                self.cluster.append(self.word_vec[i])
            else:
                leng = len(self.centroid[index])
                self.centroid[index].extend([i])
                self.cluster[index] = (leng * self.cluster[index] + self.word_vec[i]) / (leng + 1)

    def display(self):
        for item in self.centroid:
            print item


if __name__ == '__main__':
    singlepass = SinglePass(10)
    singlepass.load_training_data('single.txt')
    singlepass.learn()
    singlepass.display()


