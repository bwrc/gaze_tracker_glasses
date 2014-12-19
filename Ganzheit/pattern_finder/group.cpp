#include "group.h"
#include <stdio.h>

#include <list>


namespace group {


    /***************************************************************************
     * File specific variables
     ***************************************************************************/

    static const double PI		= 3.14159265;
    static const double TWO_PI	= 2.0*PI;

    static const double RAD_20	= (20.0  * PI)	/ 180.0;
    static const double RAD_30	= (30.0  * PI)	/ 180.0;
    static const double RAD_90	= (90.0  * PI)	/ 180.0;
    static const double RAD_180	=  PI;
    static const double RAD_270	= (270.0 * PI)	/ 180.0;


    /* Describes the state in the vacancy table */
    static const int VACANCY_OCCUPIED	= 0;
    static const int VACANCY_FREE		= 1;


    /* Functions for sorting */
    static bool upfirst(Element i, Element j)		{return (i.p.y < j.p.y);}
    static bool bottomfirst(Element i, Element j)	{return (i.p.y > j.p.y);}

    static bool fnct_min_config(Configuration &c1, Configuration &c2) {return c1.err < c2.err;}

    bool sort_elements(const Element &el1, const Element &el2) {
        return el1.index_in_orig_list < el2.index_in_orig_list;
    }


    double DEGTORAD(double deg) {
        return (deg * PI) / 180.0;
    }



    /*
     *
     *                 0
     *                 o
     *
     *
     *      1 o                o 5
     *
     *      2 o                o 4
     *
     *
     *                 o
     *                 3
     *
     * Constructor for the element
     * Parameters:
     *     _p                 : coordinate of this element
     *     _label             : the label of this element, 0 - 5, see the figure above
     *     _index_in_origlist : stores the index corresponding to the given vector in
     *                          identify(). This enables for mapping this element to
     *                          a given member in that list 
     */
    Element::Element(cv::Point _p, int _label, int _index_in_orig_list) {
        p = _p;
        label = label_init = _label;
        index_in_orig_list = _index_in_orig_list;
    }


    /*
     * Sets this element to the inital place
     */
    void Element::reset() {
        label = label_init;
    }


    /*
     * The default constructor for the Group
     */
    Group::Group() {
        id = startLabel = nof_config = config = 0;
    }


    /*
     * A specific constructor for the Group. It initialises the group according to
     * the given variables. Makes a coy of the list.
     */
    Group::Group(std::vector<Element> &l, int _id, int _startLabel) {

        members = l;
        id = _id;
        startLabel = _startLabel;
        nof_config = 0;
        config = 0;

    }


    void Group::initialiseGroupConfigurations() {

        size_t nMembers = members.size();


        /******************************************************
         * Initialise the members
         ******************************************************/

        int ind = startLabel;

        for(size_t i = 0; i < nMembers; ++i) {

            members[i].label = ind;
            members[i].label_init = ind;
            ind = (ind + 1) % 6;

        }


        positions.resize(nMembers);

        if(nMembers > 0) {

            /*
             * The number of configurations in a group depends on
             * the number of members in the group in the following way:
             *
             *   member count | number of configurations
             *   -----------------------------------------
             *    0           |  0
             *    1           |  3
             *    2           |  3
             *    3           |  1
             */
            size_t list[4] = {0, 3, 3, 1};
            nof_config = list[nMembers];

            // resize the positions accordingly
            for(size_t i = 0; i < nMembers; ++i) {

                positions[i].resize(nof_config);

            }

            /**********************************************************
             * Set the configurations in this group
             **********************************************************/
            switch(nMembers) {

                case 1: {

                    int ind = startLabel;

                    // member 1
                    positions[0][0] = ind % 6;        // cannot actually go past 5
                    positions[0][1] = (ind + 1) % 6;  // cannot actually go past 5
                    positions[0][2] = (ind + 2) % 6;

                    break;

                }

                case 2: {

                    int ind = startLabel;

                    // member 1
                    positions[0][0] = ind;
                    positions[0][1] = ind;
                    positions[0][2] = (ind + 1) % 6; // cannot actually go past 5

                    // member 2
                    positions[1][0] = (ind + 1) % 6; // cannot actually go past 5
                    positions[1][1] = (ind + 2) % 6;
                    positions[1][2] = (ind + 2) % 6;


                    break;

                }

                case 3: {

                    int ind = startLabel;

                    // member 1
                    positions[0][0] = ind;

                    // member 2
                    positions[1][0] = (ind + 1) % 6; // cannot actually go past 5

                    // member 3
                    positions[2][0] = (ind + 2) % 6;

                    break;

                }

                default: break;

            }

        }

    }


    bool Group::nextConfiguration() {

        if(config + 1 >= nof_config) {
            return false;
        }

        ++config;

        for(size_t i = 0; i < members.size(); ++i) {
            members[i].label = positions[i][config];
        }

        return true;

    }


    bool Group::prevConfiguration() {

        if(config - 1 < 0) {
            return false;
        }

        --config;

        for(size_t i = 0; i < members.size(); ++i) {
            members[i].label = positions[i][config];
        }


        return true;

    }


    void Group::initConfiguration() {

        for(size_t i = 0; i < members.size(); ++i) {
            members[i].reset();
        }

        config = 0;

    }


    bool GroupManager::assignToGroups(std::vector<cv::Point> &extracted,
                                      cv::Point pupil_centre) {

        // get references to the groups
        std::vector<Element> &group1 = groups[0].getMembers();
        std::vector<Element> &group2 = groups[1].getMembers();
        std::vector<Element> &group3 = groups[2].getMembers();
        std::vector<Element> &group4 = groups[3].getMembers();

        // how many extracted points
        size_t sz = extracted.size();

        // loop through the points and assign to correct groups
        for(size_t i = 0; i < sz; ++i) {

            const cv::Point &p = extracted[i];

            // left of pupil, groups 1 and 2
            if(p.x <= pupil_centre.x) {

                if(p.y <= pupil_centre.y) {
                    // group 1
                    group1.push_back(Element(p, -1, i));
                }
                else {
                    // group 2
                    group2.push_back(Element(p, -1, i));
                }

            }

            // right of pupil, groups 3 and 4
            else {

                if(p.y > pupil_centre.y) {
                    // group 3
                    group3.push_back(Element(p, -1, i));
                }
                else {
                    // group 4
                    group4.push_back(Element(p, -1, i));
                }

            }

        }

        // sanity check, a group can have a max of 3 elements
        if(groups[0].size() > 3 ||
           groups[1].size() > 3 ||
           groups[2].size() > 3 ||
           groups[3].size() > 3) {

            return false;

        }

        return true;

    }


    /*	sort the points like so
     *
     *                 0
     *                 o
     *
     *
     *      1 o                o 5
     *
     *      2 o                o 4
     *
     *
     *                 o
     *                 3
     *
     */
    void GroupManager::sortGroups() {

        std::sort(groups[0].getMembers().begin(), groups[0].getMembers().end(), upfirst);
        std::sort(groups[1].getMembers().begin(), groups[1].getMembers().end(), upfirst);

        std::sort(groups[2].getMembers().begin(), groups[2].getMembers().end(), bottomfirst);
        std::sort(groups[3].getMembers().begin(), groups[3].getMembers().end(), bottomfirst);

    }


    void GroupManager::initialiseGroups() {

        /*************************************************************************
         * Set the positions. In these positions there is no overlap between
         * the group members within the group. However, this does not mean that
         * there is no overlap between members of adjacent groups.
         *************************************************************************/

        std::vector<Group>::iterator git = groups.begin();

        while(git != groups.end()) {

            git->initialiseGroupConfigurations();

            ++git;

        }

    }


    bool GroupManager::identify(std::vector<cv::Point> &extracted,
                                cv::Point pupil_centre) {

        nof_points = (int)extracted.size();
        if(nof_points <= 1 || nof_points >= 7) {
            return false;
        }

        // clear any previous configurations
        configurations.clear();

        // allocate space for the groups
        groups.resize(4);

        // initialise the groups and set the group IDs
        std::vector<Element> v;
        groups[0] = Group(v, 0, 0);
        groups[1] = Group(v, 1, 1);
        groups[2] = Group(v, 2, 3);
        groups[3] = Group(v, 3, 4);


        // assign the points to their groups
        if(!assignToGroups(extracted, pupil_centre)) {
            return false;
        }

        // sort the group members
        sortGroups();


        // set the group configurations
        initialiseGroups();

        /****************************************************
         * Loop through all configurations and store the
         * valid ones.
         ****************************************************/
        std::vector<Group>::iterator git = groups.begin();
        loopConfigurations(git);

        if(configurations.size() == 0) {
            return false;
        }
        // get the configuration with the minimum error
        std::vector<Configuration>::iterator min_config = std::min_element(configurations.begin(),
                                                                           configurations.end(),
                                                                           fnct_min_config);

        config_best = *min_config;

        return true;

    }


    void GroupManager::loopConfigurations(std::vector<Group>::iterator git) {

        std::vector<Group>::iterator git_next = git + 1;

        do {

            /*
             * std::vector::end() "Returns an iterator referring
             * to the past-the-end element in the vector container."
             */
            if(git_next != groups.end()) {
                loopConfigurations(git_next);
            }

            // if this is the last group, i.e. group4
            if(git == groups.end() - 1) {

                //                print(groups);

                if(isConfigOk()) {
                    //                    printf("ok\n");

                    storeConfiguration();

                }

            }

            // if this is the first configuration, i.e. group1
            if(git == groups.begin()) {


            }

        }
        /*
         * Loop through the configurations.
         */
        //        while(nextConfiguration(git) == MSG_CONFIGURATION_OK);
        while(git->nextConfiguration());

        // set the initial configuration
        git->initConfiguration();

    }


    void GroupManager::storeConfiguration() {

        // compute the error of the current configuration
        const double err = computeError();

        Configuration c(err);


        // get a reference to it
        //       Configuration &c = configurations.back();

        // get a reference to the label list
        std::vector<Element> &elements = c.elements;


        /*************************************************
         * Copy all elements from all groups to the list.
         *************************************************/

        std::vector<Group>::const_iterator git = groups.begin();

        elements.resize(nof_points);
        std::vector<Element>::iterator lit = elements.begin();

        for(; git < groups.end(); ++git) {

            const std::vector<Element> &members = git->getMembers();
            std::vector<Element>::const_iterator mit = members.begin();

            for(; mit < members.end(); ++mit) {
                *lit++ = *mit;
            }

        }


        /*
         * Sort ascending according to index_in_orig_list, i.e. the 
         * elements are in the same order as
         * std::vector<cv::Point> &extracted in identify()
         */
        std::sort(elements.begin(), elements.end(), sort_elements);

        // place it in the list
        configurations.push_back(c);

    }


    double GroupManager::computeError() {

        /*
         *               0
         *               o
         *
         *      1 o             o 5
         *
         *      2 o             o 4
         *
         *               o
         *               3
         */

        // absolute angle between 2 to 3, for example
        static const double tmp1 = 26.5650512;

        // absolute angle between 3 to 4, for example
        static const double tmp2 = 75.9637565;

        static const double angles[6*5] = {

            /* 0 to others */
            DEGTORAD(-(90 + (90 - tmp1))),	// 0 to 1
            DEGTORAD(-(90 + 45)),			// 0 to 2
            DEGTORAD(-90),					// 0 to 3
            DEGTORAD(-45),					// 0 to 4
            DEGTORAD(-tmp1),				// 0 to 5

            /* 1 to others */
            DEGTORAD(-90),					// 1 to 2
            DEGTORAD(-45),					// 1 to 3
            DEGTORAD(-(90 - tmp2)),			// 1 to 4
            DEGTORAD(0),					// 1 to 5
            DEGTORAD(tmp1),					// 1 to 0

            /* 2 to others */
            DEGTORAD(-tmp1),				// 2 to 3
            DEGTORAD(0),					// 2 to 4
            DEGTORAD(90 - tmp2),			// 2 to 5
            DEGTORAD(45),					// 2 to 0
            DEGTORAD(90),					// 2 to 1

            /* 3 to others */
            DEGTORAD(tmp1),					// 3 to 4
            DEGTORAD(45),					// 3 to 5
            DEGTORAD(90),					// 3 to 0
            DEGTORAD(135),					// 3 to 1
            DEGTORAD(90 + (90 - tmp1)),		// 3 to 2

            /* 4 to others */
            DEGTORAD(90),					// 4 to 5
            DEGTORAD(135),					// 4 to 0
            DEGTORAD(90 + tmp2),			// 4 to 1
            DEGTORAD(180),					// 4 to 2
            DEGTORAD(-(90 + (90 - tmp1))),	// 4 to 3

            /* 5 to others */
            DEGTORAD(90 + (90 - tmp1)),		// 5 to 0
            DEGTORAD(180),					// 5 to 1
            DEGTORAD(-(90 + tmp2)),			// 5 to 2
            DEGTORAD(-(90 + 45)),			// 5 to 3
            DEGTORAD(-90)					// 5 to 4

        };

        /************************************************************
         * Get all elements from all groups
         *************************************************************/
        std::vector<const Element *> elements;

        std::vector<Group>::const_iterator git = groups.begin();

        for(; git != groups.end(); ++git) {

            const std::vector<Element> &m = git->getMembers();

            size_t msz = m.size();

            for(size_t i = 0; i < msz; ++i) {
                const Element *e = (Element *)&(m[i]);

                elements.push_back(e);
            }

        }


        /************************************************************
         * Compute the error
         *************************************************************/
        double err_sum = 0.0;

        // must always be nof_points
        const size_t sz = elements.size();

        for(size_t i = 0; i < sz; ++i) {

            // take an element...
            const Element *el1 = elements[i];

            size_t j = (i + 1) % sz;

            // ...and compare its location to the other elements
            while(j != i) {

                // compare this to el1
                const Element *el2 = elements[j];

                /*
                 * Difference in x and y directions. Note that y has been
                 * flipped, because the positive rotation direction is
                 * counterclockwisem, i.e. positive x is to the right and
                 * positive y is upwards.
                 */
                const int diffy = -(el2->p.y - el1->p.y);
                const int diffx = el2->p.x - el1->p.x;
                double ang = 0;

                // diffx and diffy cannot both be zeros simultaneously
                if(diffx == 0) {
                    ang = diffy > 0 ? RAD_90 : -RAD_90;
                }
                else if(diffy == 0) {
                    ang = diffx > 0 ? 0 : RAD_180;
                }
                else {
                    ang = atan2(diffy, diffx);
                }

                const int ind1 = el1->label;
                const int ind2 = el2->label == 0 ? 6 : el2->label;
                int ind_diff = ind2 - ind1;
                ind_diff = ind_diff < 0 ? 6 + ind_diff : ind_diff;

                const double ang_real = angles[el1->label*5 + ind_diff - 1];
                const double ang_diff = ang_real - ang;


                /*
                 * Consider the case:
                 *     ang		= 175
                 *     ang_real	= -175
                 *
                 * The difference is 10, not 350
                 */
                const double err = std::min(std::abs(ang_diff), std::abs(TWO_PI - ang_diff));

                err_sum += err*err;


                j = (j + 1) % sz;

            }

        }

        return err_sum;

    }


    bool GroupManager::testVacancy(const int table[6], const std::vector<Element> &elements) {

        size_t sz = elements.size();

        for(size_t i = 0; i < sz; ++i) {

            if(table[elements[i].label] == VACANCY_OCCUPIED) {
                return false;
            }

        }

        return true;

    }

    void GroupManager::addToVacancyTable(const std::vector<Element> &elements, int table[6]) {

        size_t sz = elements.size();

        for(size_t i = 0; i < sz; ++i) {
            table[elements[i].label] = VACANCY_OCCUPIED;
        }

    }


    bool GroupManager::isConfigOk() {

        /************************************************
         * Check that N's last member is behind group
         * (N+1)'s first member
         ************************************************/

        {
            std::vector<Group>::iterator git = groups.begin();
            std::vector<Group>::iterator gitLast = groups.end() - 1; // group4
            std::vector<Group>::iterator gitFirst = groups.begin(); // group1


            for( ; git != groups.end(); ++git) {

                if(git->getMembers().size() == 0) {
                    continue;
                }


                // the next group. If the current is group0, the next is group4
                std::vector<Group>::iterator gitNext = git != (groups.end() - 1) ? (git + 1) : groups.begin();

                if(gitNext->getMembers().size() == 0) {
                    continue;
                }


                /*****************************************************
                 * Check if the last label in this group exceeds
                 * the next group's first label
                 *****************************************************/

                int ind_last_in_this = git->getMembers().back().label;
                int ind_first_in_next = gitNext->getMembers().front().label;

                if(git == gitLast) {

                    if(ind_last_in_this == 0) {
                        ind_last_in_this = 6;
                    }

                }
                if(gitNext == gitLast) {

                    if(ind_first_in_next == 0) {
                        ind_first_in_next = 6;
                    }

                }

                if(gitNext == gitFirst) {
                    ind_first_in_next += 6;
                }


                if(ind_last_in_this >= ind_first_in_next) {
                    return false;
                }


            }

        }


        /****************************************************
         * Now check overlap. TODO: is overlap actually ok?
         ****************************************************/

        int cpVacancy[6] = {VACANCY_FREE, VACANCY_FREE, VACANCY_FREE, VACANCY_FREE, VACANCY_FREE, VACANCY_FREE};

        addToVacancyTable(groups[0].getMembers(), cpVacancy);

        std::vector<Group>::iterator git = groups.begin() + 1;

        while(git != groups.end()) {

            const std::vector<Element> &members = git->getMembers();

            if(!GroupManager::testVacancy(cpVacancy, members)) {
                return false;
            }

            GroupManager::addToVacancyTable(members, cpVacancy);

            ++git;

        }


        return true;

    }


} // end of "namespace group {

