import math
import random
import numpy as np

#from decimal import *
#getcontext().rounding = ROUND_DOWN
#print(getcontext())
#.prec = 4

def truncate(x):
    return int(x * 10000) / 10000

class User:
    def __init__(self, name, e_bal, vo_bal):
        self.name = name
        self.e_bal = e_bal
        self.vo_bal = vo_bal
        self.evo_bal = 0

    def state(self):
        print(self.name, self.e_bal, self.vo_bal, self.evo_bal)

    def add_e_balance(self, x):
        if (self.e_bal + x < 0):
            print("insufficient balance")
            exit
        self.e_bal = truncate(self.e_bal + x)

    def add_vo_balance(self, x):
        if (self.vo_bal + x < 0):
            print("insufficient balance")
            exit
        self.vo_bal = truncate(self.vo_bal + x)

    def add_evo_balance(self, x):
        if (self.evo_bal + x < 0):
            print("insufficient balance")
            exit
        self.evo_bal = truncate(self.evo_bal + x)


class Evodex:
    def __init__(self, user, e_bal, vo_bal, evo_minted, e_weight, vo_weight, fee):
        user.add_e_balance(-e_bal)
        user.add_vo_balance(-vo_bal)
        user.add_evo_balance(evo_minted)
        self.e_weight = e_weight
        self.vo_weight = vo_weight
        self.fee = fee
        self.e_bal = e_bal
        self.vo_bal = vo_bal
        self.evo_minted = evo_minted
        self.state_parameter = pow(self.e_bal, self.e_weight) * pow(self.vo_bal, self.vo_weight) / self.evo_minted
    
#    def state_parameter(self):
 #       return pow(self.e_bal, self.e_weight) * pow(self.vo_bal, self.vo_weight) / self.evo_minted

    def buytoken(self, user, e, vo, min_expected):
        if (e < 0) | (vo < 0) | (min_expected < 0):
            print("input must be positive")
            return
        print(user.name, "buytoken", e, vo, min_expected)
        self.operate_token(user, e, vo, min_expected)

    def selltoken(self, user, e, vo, max_expected):
        if (e < 0) | (vo < 0) | (max_expected < 0):
            print("input must be positive")
            return
        print(user.name, "selltoken", e, vo, max_expected)
        self.operate_token(user, -e, -vo, -max_expected)

    def exchange(self, user, e, vo):
        print(user.name, 'exchange', e, vo)
        if ((e >= 0) & (vo >= 0)) | ((e <= 0) & (vo <= 0)):
            print("one must be positive and the other negative")
            return
        C1 = self.e_bal
        C2 = self.vo_bal
        C1_in = e
        C2_in = vo
        W1 = self.e_weight
        W2 = self.vo_weight
        if (C1_in > 0):
            C1_in = truncate( (C1_in * (10000 - self.fee)) / 10000)
        if (C2_in > 0):
            C2_in = truncate( (C2_in * (10000 - self.fee)) / 10000)
        final = pow( C1 + C1_in, W1) * pow( C2 + C2_in, W2)
        initial = pow(C1, W1) * pow(C2, W2)
        if (final < initial):
            print("available is less than expected")
            return
        user.add_e_balance(-e)
        user.add_vo_balance(-vo)
        self.e_bal = truncate(self.e_bal + e)
        self.vo_bal = truncate(self.vo_bal + vo)


    def operate_token(self, user, e, vo, min_expected):
        A = self.evo_minted
        C1 = self.e_bal
        C2 = self.vo_bal
        C1_in = e
        C2_in = vo
        W1 = self.e_weight
        W2 = self.vo_weight
        if (C1_in > 0):
            C1_in = truncate( (C1_in * (10000 - self.fee)) / 10000)
        if (C2_in > 0):
            C2_in = truncate( (C2_in * (10000 - self.fee)) / 10000)
        tok_out = truncate(A * pow( (C1+C1_in) / C1, W1 ) * pow( (C2+C2_in) / C2, W2 ) - A - 0.0001)
        if (tok_out < min_expected):
            print("available is less than expected")
            return
        # tok_out = truncate(tok_out - 0.0001)
        # print("fee: ", e - C1_in, "EOS", vo - C2_in, "VOICE")
        print("tok_out:", tok_out)
        user.add_e_balance(-e)
        user.add_vo_balance(-vo)
        user.add_evo_balance(tok_out)
        self.e_bal = truncate(self.e_bal + e)
        self.vo_bal = truncate(self.vo_bal + vo)
        self.evo_minted = truncate(self.evo_minted + tok_out)

    def state(self):
        print("Evodex: ", self.e_bal, self.vo_bal, self.evo_minted)
        self.state_parameter =  pow(self.e_bal, self.e_weight) * pow(self.vo_bal, self.vo_weight) / self.evo_minted
        print("Characteristic number:", self.state_parameter, "\n") # pow(self.e_bal, self.e_weight) * pow(self.vo_bal, self.vo_weight) / self.evo_minted, "\n" )


alice = User('alice', 1000, 20000)
alice.state()

evodex = Evodex(alice, 100, 10000, 1000, .5, .5, 10)
evodex.state()
alice.state()

print('1')
evodex.buytoken(alice, 50, 0, 1)
alice.state()
evodex.state()

print('2')
evodex.selltoken(alice, 49.951, 0, 1000)   # Alice puede recuperar un poco de la fee que pagó
alice.state()
evodex.state()

print('3')
evodex.buytoken(alice, 101.1313, 103.0001, 1)
alice.state()
evodex.state()

print('4')
evodex.selltoken(alice, 3, 100, 1000)
alice.state()
evodex.state()

print('5')
evodex.exchange(alice, 4, -10)
alice.state()
evodex.state()

print('6')
evodex.exchange(alice, 0.1, -4.85)
alice.state()
evodex.state()

def fee(x):
    print(x, " luego de fee: ", truncate( (x * (10000 - 10)) / 10000))
fee(10)
fee(1)
fee(.1)
fee(.01)
fee(.001)
fee(.0001)
fee(0.32)