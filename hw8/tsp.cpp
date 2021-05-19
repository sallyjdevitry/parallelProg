#include <iostream>
#include <mpi.h>
#include <unistd.h>
#include <stdlib.h>
#include <vector>
#include <math.h>

using namespace std;

#define MCW MPI_COMM_WORLD

#define numCities 100
#define popSize 10

int cities[numCities][2] = {{179140, 750703}, {78270, 737081}, {577860, 689926}, {628150, 597095}, {954030, 510314}, {837880, 811285}, {410640, 846947}, {287850, 600161}, {270030, 494359}, {559020, 199445}, {353930, 542989}, {515920, 497472}, {648080, 470280}, {594550, 968799}, {386690, 907669}, {93070, 395385}, {93620, 313966}, {426870,  39662}, {437000, 139949}, {789810, 488001}, {749130, 575522}, {481030, 286118}, {670720, 392925}, {273890, 892877}, {138430, 562658}, {85480, 465869}, {775340, 220065}, {862980, 312238}, {155180, 263662}, {274070, 74689}, {333340, 456245}, {822150, 399803}, {158880, 612518}, {815560, 707417}, {678240, 709341}, {394470, 679221}, {631300, 846813}, {528320, 824193}, {666940, 845130}, {298650, 816352}, {243750, 745443}, {220500, 654221}, {338920, 381007}, {313110, 201386}, {856380, 564703}, {549250, 565255}, {537400, 604425}, {502110, 435463}, {498840, 590729}, {482310, 571034}, {416930, 765126}, {418400, 638700}, {374170, 695851}, {412370, 570904}, {301090, 737412}, {235690, 782470}, {475940, 439645}, {268540, 609753}, {130500, 712663}, {81660, 732470}, {64520, 711936}, {264690, 529248}, {90230, 612484}, {38370, 610277}, {15430, 579032}, {138890, 482432}, {264580, 421188}, {86690, 394738}, {209190, 347661}, {425890, 376154}, {312480, 177450}, {373360, 142350}, {442850, 106198}, {505100, 189757}, {542610, 224170}, {566730, 262940}, {615970, 237922}, {612120, 303181}, {634410, 320152}, {879480, 239867}, {868760, 286928}, {807670, 334613}, {943060, 368070}, {827280, 387076}, {896040, 413699}, {920900, 454842}, {746380, 440559}, {734300, 452247}, {730780, 471211}, {870570, 549620}, {607060, 453077}, {926580, 669624}, {812660, 614479}, {701420, 559132}, {688600, 580646}, {743800, 669521}, {819700, 857004}, {683690, 682649}, {732680, 857362}, {685760,  866857}};

// string gene defines the path traversed by the salesperson
struct order {
  string gene;
  int fitness;
};

int rand_num(int start, int end) {
  int r = end - start;
  int rnum = start + rand() % r;
  return rnum;
}

// Mutated gene is a string with a random swap of two cities to create variation
string mutatedGene(string gene) {
  while (true) {
    int r = rand_num(1, numCities);
    int r1 = rand_num(1, numCities);
    if (r1 != r) {
      char temp = gene[r];
      gene[r] = gene[r1];
      gene[r1] = temp;
      break;
    }
  }
  return gene;
}

//check if character has already appeared in string
bool isRepeat(string s, char ch) {
  for (int i = 0; i < s.size(); i++)
  {
    if (s[i] == ch)
      return true;
  }
  return false;
}

//return a valid gene string required to create the population
string createGene() {
  string gene = "0";
  while (true)
  {
    if (gene.size() == numCities)
    {
      gene += gene[0];
      break;
    }
    int temp = rand_num(1, numCities);
    if (!isRepeat(gene, (char)(temp + 48)))
      gene += (char)(temp + 48);
  }
  return gene;
}

//return the fitness value of a gene. The fitness value is the euclidean distance
int getFitness(string gene){
  int eucDist = 0;
  for (int i = 0; i < gene.size() - 1; i++)
  {
    eucDist += sqrt((pow(int(cities[gene[i]-48][0])-int(cities[gene[i+1]-48][0]), 2)) + (pow(int(cities[gene[i]-48][1])-int(cities[gene[i+1]-48][1]), 2)));
  }
  return eucDist;
}

// Comparator for gene struct.
bool lessThan(struct order t1, struct order t2) {
  return t1.fitness < t2.fitness;
}

int cool(int temp) {
  return (92 * temp) / 100;
}

void runTSP(int rank) {
  struct order bestYet;
  bestYet.gene = "0";
  bestYet.fitness = 10000000000;
  int stopper = 0;
  int gen = 1;
  // stop at this number of gene iterations
  int maxGen = 1000;

  vector<struct order> population;
  struct order temp;

  int temperature = 1000000000;

  if (rank == 0) {
    // populate the gene pool.
    for (int i = 0; i < popSize; i++) {
      temp.gene = createGene();
      temp.fitness = getFitness(temp.gene);
      if (temp.fitness < bestYet.fitness) {
        bestYet.fitness = temp.fitness;
      }
      population.push_back(temp);
    }

    cout << "\nInitial population: " << endl
        << "gene     FITNESS VALUES\n";
    for (int i = 0; i < popSize; i++)
      cout << population[i].fitness << endl;
    cout << "\n";
  }

  // MPI_Barrier(MCW);

  // perform crossing and gene mutation.
  while (gen <= maxGen) {
    sort(population.begin(), population.end(), lessThan);
    vector<struct order> new_population;

    for (int i = 0; i < popSize; i++) {
      struct order p1 = population[i];

      while (true) {
        if (stopper > 1000000) {
          break;
        }
        string new_g = mutatedGene(p1.gene);
        struct order new_gene;
        new_gene.gene = new_g;
        new_gene.fitness = getFitness(new_gene.gene);

        if (new_gene.fitness <= population[i].fitness) {
          if (new_gene.fitness < bestYet.fitness) {
            bestYet.fitness = new_gene.fitness;
          }
          new_population.push_back(new_gene);
          break;
        }
        else {
          // Accept the bad children at a probablity
          float prob = pow(2.7, -1 * ((float)(new_gene.fitness - population[i].fitness) / temperature));
          if (prob > 0.3) {
            if (new_gene.fitness < bestYet.fitness) {
              bestYet.fitness = new_gene.fitness;
            }
            new_population.push_back(new_gene);
            break;
          }
        }
        stopper++;
      }
    }

    temperature = cool(temperature);
    population = new_population;
    cout << "\nGeneration " << gen << " \n";
    cout << "FITNESS VALUES\n";

    for (int i = 0; i < popSize; i++) {
      cout << population[i].fitness << endl;
    }
    gen++;
    cout << "BEST PATH LENGTH = " << bestYet.fitness << endl;
  }
}

int main(int argc, char **argv) {
  int rank, size;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MCW, &rank);
  MPI_Comm_size(MCW, &size);
  MPI_Request request;

  if (rank == 0) {
    runTSP(rank);
  }
  MPI_Finalize();
  return 0;
}
