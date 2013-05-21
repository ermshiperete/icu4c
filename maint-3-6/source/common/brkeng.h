/**
 *******************************************************************************
 * Copyright (C) 2006, International Business Machines Corporation and others. *
 * All Rights Reserved.                                                        *
 *******************************************************************************
 */

#ifndef BRKENG_H
#define BRKENG_H

#include "unicode/utypes.h"
#include "unicode/uobject.h"
#include "unicode/utext.h"

U_NAMESPACE_BEGIN

class UnicodeSet;
class UStack;

/*******************************************************************
 * LanguageBreakEngine
 */

/**
 * <p>LanguageBreakEngines implement language-specific knowledge for
 * finding text boundaries within a run of characters belonging to a
 * specific set. The boundaries will be of a specific kind, e.g. word,
 * line, etc.</p>
 *
 * <p>LanguageBreakEngines should normally be implemented so as to
 * be shared between threads without locking.</p>
 */
class LanguageBreakEngine : public UMemory {
 public:

  /**
   * <p>Default constructor.</p>
   *
   */
  LanguageBreakEngine();

  /**
   * <p>Virtual destructor.</p>
   */
  virtual ~LanguageBreakEngine();

 /**
  * <p>Indicate whether this engine handles a particular character for
  * a particular kind of break.</p>
  *
  * @param c A character which begins a run that the engine might handle
  * @param breakType The type of text break which the caller wants to determine
  * @return TRUE if this engine handles the particular character and break
  * type.
  */
  virtual UBool handles(UChar32 c, int32_t breakType) const = 0;

 /**
  * <p>Find any breaks within a run in the supplied text.</p>
  *
  * @param text A UText representing the text. The
  * iterator is left at the end of the run of characters which the engine
  * is capable of handling.
  * @param startPos The start of the run within the supplied text.
  * @param endPos The end of the run within the supplied text.
  * @param reverse Whether the caller is looking for breaks in a reverse
  * direction.
  * @param breakType The type of break desired, or -1.
  * @param foundBreaks An allocated C array of the breaks found, if any
  * @return The number of breaks found.
  */
  virtual int32_t findBreaks( UText *text,
                              int32_t startPos,
                              int32_t endPos,
                              UBool reverse,
                              int32_t breakType,
                              UStack &foundBreaks ) const = 0;

};

/*******************************************************************
 * LanguageBreakFactory
 */

/**
 * <p>LanguageBreakFactorys find and return a LanguageBreakEngine
 * that can determine breaks for characters in a specific set, if
 * such an object can be found.</p>
 *
 * <p>If a LanguageBreakFactory is to be shared between threads,
 * appropriate synchronization must be used; there is none internal
 * to the factory.</p>
 *
 * <p>A LanguageBreakEngine returned by a LanguageBreakFactory can
 * normally be shared between threads without synchronization, unless
 * the specific subclass of LanguageBreakFactory indicates otherwise.</p>
 *
 * <p>A LanguageBreakFactory is responsible for deleting any LanguageBreakEngine
 * it returns when it itself is deleted, unless the specific subclass of
 * LanguageBreakFactory indicates otherwise. Naturally, the factory should
 * not be deleted until the LanguageBreakEngines it has returned are no
 * longer needed.</p>
 */
class LanguageBreakFactory : public UMemory {
 public:

  /**
   * <p>Default constructor.</p>
   *
   */
  LanguageBreakFactory();

  /**
   * <p>Virtual destructor.</p>
   */
  virtual ~LanguageBreakFactory();

 /**
  * <p>Find and return a LanguageBreakEngine that can find the desired
  * kind of break for the set of characters to which the supplied
  * character belongs. It is up to the set of available engines to
  * determine what the sets of characters are.</p>
  *
  * @param c A character that begins a run for which a LanguageBreakEngine is
  * sought.
  * @param breakType The kind of text break for which a LanguageBreakEngine is
  * sought.
  * @return A LanguageBreakEngine with the desired characteristics, or 0.
  */
  virtual const LanguageBreakEngine *getEngineFor(UChar32 c, int32_t breakType) = 0;

};

/*******************************************************************
 * UnhandledEngine
 */

/**
 * <p>UnhandledEngine is a special subclass of LanguageBreakEngine that
 * handles characters that no other LanguageBreakEngine is available to
 * handle. It is told the character and the type of break; at its
 * discretion it may handle more than the specified character (e.g.,
 * the entire script to which that character belongs.</p>
 *
 * <p>UnhandledEngines may not be shared between threads without
 * external synchronization.</p>
 */

class UnhandledEngine : public LanguageBreakEngine {
 private:

    /**
     * The sets of characters handled, for each break type
     * @internal
     */

  UnicodeSet    *fHandled[4];

 public:

  /**
   * <p>Default constructor.</p>
   *
   */
  UnhandledEngine(UErrorCode &status);

  /**
   * <p>Virtual destructor.</p>
   */
  virtual ~UnhandledEngine();

 /**
  * <p>Indicate whether this engine handles a particular character for
  * a particular kind of break.</p>
  *
  * @param c A character which begins a run that the engine might handle
  * @param breakType The type of text break which the caller wants to determine
  * @return TRUE if this engine handles the particular character and break
  * type.
  */
  virtual UBool handles(UChar32 c, int32_t breakType) const;

 /**
  * <p>Find any breaks within a run in the supplied text.</p>
  *
  * @param text A UText representing the text (TODO: UText). The
  * iterator is left at the end of the run of characters which the engine
  * is capable of handling.
  * @param startPos The start of the run within the supplied text.
  * @param endPos The end of the run within the supplied text.
  * @param reverse Whether the caller is looking for breaks in a reverse
  * direction.
  * @param breakType The type of break desired, or -1.
  * @param foundBreaks An allocated C array of the breaks found, if any
  * @return The number of breaks found.
  */
  virtual int32_t findBreaks( UText *text,
                              int32_t startPos,
                              int32_t endPos,
                              UBool reverse,
                              int32_t breakType,
                              UStack &foundBreaks ) const;

 /**
  * <p>Tell the engine to handle a particular character and break type.</p>
  *
  * @param c A character which the engine should handle
  * @param breakType The type of text break for which the engine should handle c
  */
  virtual void handleCharacter(UChar32 c, int32_t breakType);

};

/*******************************************************************
 * ICULanguageBreakFactory
 */

/**
 * <p>ICULanguageBreakFactory is the default LanguageBreakFactory for
 * ICU. It creates dictionary-based LanguageBreakEngines from dictionary
 * data in the ICU data file.</p>
 */
class ICULanguageBreakFactory : public LanguageBreakFactory {
 private:

    /**
     * The stack of break engines created by this factory
     * @internal
     */

  UStack    *fEngines;

 public:

  /**
   * <p>Standard constructor.</p>
   *
   */
  ICULanguageBreakFactory(UErrorCode &status);

  /**
   * <p>Virtual destructor.</p>
   */
  virtual ~ICULanguageBreakFactory();

 /**
  * <p>Find and return a LanguageBreakEngine that can find the desired
  * kind of break for the set of characters to which the supplied
  * character belongs. It is up to the set of available engines to
  * determine what the sets of characters are.</p>
  *
  * @param c A character that begins a run for which a LanguageBreakEngine is
  * sought.
  * @param breakType The kind of text break for which a LanguageBreakEngine is
  * sought.
  * @return A LanguageBreakEngine with the desired characteristics, or 0.
  */
  virtual const LanguageBreakEngine *getEngineFor(UChar32 c, int32_t breakType);

};

U_NAMESPACE_END

    /* BRKENG_H */
#endif